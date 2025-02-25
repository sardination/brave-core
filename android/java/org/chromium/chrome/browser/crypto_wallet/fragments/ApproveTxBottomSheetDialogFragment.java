/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.fragments;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Looper;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.webkit.URLUtil;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;
import androidx.viewpager.widget.ViewPager;

import com.google.android.material.bottomsheet.BottomSheetDialogFragment;
import com.google.android.material.tabs.TabLayout;

import org.chromium.base.Log;
import org.chromium.brave_wallet.mojom.AccountInfo;
import org.chromium.brave_wallet.mojom.AssetRatioService;
import org.chromium.brave_wallet.mojom.BlockchainRegistry;
import org.chromium.brave_wallet.mojom.BlockchainToken;
import org.chromium.brave_wallet.mojom.BraveWalletService;
import org.chromium.brave_wallet.mojom.JsonRpcService;
import org.chromium.brave_wallet.mojom.KeyringService;
import org.chromium.brave_wallet.mojom.NetworkInfo;
import org.chromium.brave_wallet.mojom.TransactionInfo;
import org.chromium.brave_wallet.mojom.TransactionType;
import org.chromium.brave_wallet.mojom.TxService;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.crypto_wallet.activities.BraveWalletBaseActivity;
import org.chromium.chrome.browser.crypto_wallet.adapters.ApproveTxFragmentPageAdapter;
import org.chromium.chrome.browser.crypto_wallet.listeners.TransactionConfirmationListener;
import org.chromium.chrome.browser.crypto_wallet.observers.ApprovedTxObserver;
import org.chromium.chrome.browser.crypto_wallet.util.ParsedTransaction;
import org.chromium.chrome.browser.crypto_wallet.util.SolanaTransactionsGasHelper;
import org.chromium.chrome.browser.crypto_wallet.util.TokenUtils;
import org.chromium.chrome.browser.crypto_wallet.util.TransactionUtils;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;
import org.chromium.chrome.browser.util.TabUtils;
import org.chromium.url.GURL;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ApproveTxBottomSheetDialogFragment extends BottomSheetDialogFragment {
    public static final String TAG_FRAGMENT = ApproveTxBottomSheetDialogFragment.class.getName();

    private TransactionInfo mTxInfo;
    private String mAccountName;
    private boolean mRejected;
    private boolean mApproved;
    private ApprovedTxObserver mApprovedTxObserver;
    private ExecutorService mExecutor;
    private Handler mHandler;
    private TransactionConfirmationListener mTransactionConfirmationListener;
    private List<TransactionInfo> mTransactionInfos;
    private Button mRejectAllTx;
    private int mCoinType;
    private long mSolanaEstimatedTxFee;

    public static ApproveTxBottomSheetDialogFragment newInstance(
            List<TransactionInfo> transactionInfos, TransactionInfo txInfo, String accountName,
            TransactionConfirmationListener listener) {
        return new ApproveTxBottomSheetDialogFragment(
                transactionInfos, txInfo, accountName, listener);
    }

    public static ApproveTxBottomSheetDialogFragment newInstance(
            TransactionInfo txInfo, String accountName) {
        List<TransactionInfo> infos = new ArrayList<>();
        infos.add(txInfo);
        return newInstance(infos, txInfo, accountName, null);
    }

    private ApproveTxBottomSheetDialogFragment(TransactionInfo txInfo, String accountName) {
        mTxInfo = txInfo;
        mAccountName = accountName;
        mRejected = false;
        mApproved = false;
        mExecutor = Executors.newSingleThreadExecutor();
        mHandler = new Handler(Looper.getMainLooper());
        // TODO (Wengling): To support other networks, all hard-coded chainSymbol, etc. need to be
        // get from current network instead.
        mTransactionInfos = Collections.emptyList();
        mSolanaEstimatedTxFee = 0;
    }

    ApproveTxBottomSheetDialogFragment(List<TransactionInfo> transactionInfos,
            TransactionInfo txInfo, String accountName,
            @Nullable TransactionConfirmationListener transactionConfirmationListener) {
        this(txInfo, accountName);
        mTransactionInfos = transactionInfos;
        mTransactionConfirmationListener = transactionConfirmationListener;
    }

    public void setApprovedTxObserver(ApprovedTxObserver approvedTxObserver) {
        mApprovedTxObserver = approvedTxObserver;
    }

    // TODO: Make these into an interface so they can be shared between fragments and activities
    private AssetRatioService getAssetRatioService() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletBaseActivity) {
            return ((BraveWalletBaseActivity) activity).getAssetRatioService();
        }
        return null;
    }

    private TxService getTxService() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletBaseActivity) {
            return ((BraveWalletBaseActivity) activity).getTxService();
        }
        return null;
    }

    private JsonRpcService getJsonRpcService() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletBaseActivity) {
            return ((BraveWalletBaseActivity) activity).getJsonRpcService();
        }
        return null;
    }

    private BlockchainRegistry getBlockchainRegistry() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletBaseActivity) {
            return ((BraveWalletBaseActivity) activity).getBlockchainRegistry();
        }
        return null;
    }

    private BraveWalletService getBraveWalletService() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletBaseActivity) {
            return ((BraveWalletBaseActivity) activity).getBraveWalletService();
        }
        return null;
    }

    private KeyringService getKeyringService() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletBaseActivity) {
            return ((BraveWalletBaseActivity) activity).getKeyringService();
        }
        return null;
    }

    @Override
    public void show(FragmentManager manager, String tag) {
        try {
            ApproveTxBottomSheetDialogFragment fragment =
                    (ApproveTxBottomSheetDialogFragment) manager.findFragmentByTag(
                            ApproveTxBottomSheetDialogFragment.TAG_FRAGMENT);
            FragmentTransaction transaction = manager.beginTransaction();
            if (fragment != null) {
                transaction.remove(fragment);
            }
            transaction.add(this, tag);
            transaction.commitAllowingStateLoss();
        } catch (IllegalStateException e) {
            Log.e("ApproveTxBottomSheetDialogFragment", e.getMessage());
        }
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        super.onDismiss(dialog);
        if (mApprovedTxObserver != null) {
            if (mRejected || mApproved) {
                // TODO(pav): 28/07/22 rename to callback or delegate, it's not an observer
                mApprovedTxObserver.onTxApprovedRejected(mApproved, mAccountName, mTxInfo.id);
            } else {
                mApprovedTxObserver.onTxPending(mAccountName, mTxInfo.id);
            }
        }
        if (mTransactionConfirmationListener != null) {
            mTransactionConfirmationListener.onDismiss();
        }
    }

    @Override
    public void setupDialog(@NonNull Dialog dialog, int style) {
        super.setupDialog(dialog, style);
        @SuppressLint("InflateParams")
        final View view =
                LayoutInflater.from(getContext()).inflate(R.layout.approve_tx_bottom_sheet, null);
        dialog.setContentView(view);
        ViewParent parent = view.getParent();
        ((View) parent).getLayoutParams().height = ViewGroup.LayoutParams.WRAP_CONTENT;
        JsonRpcService jsonRpcService = getJsonRpcService();
        KeyringService keyringService = getKeyringService();
        assert jsonRpcService != null && keyringService != null;

        TextView networkName = view.findViewById(R.id.network_name);
        TextView txType = view.findViewById(R.id.tx_type);
        if (mTxInfo.txType
                == TransactionType
                           .SOLANA_SPL_TOKEN_TRANSFER_WITH_ASSOCIATED_TOKEN_ACCOUNT_CREATION) {
            TextView associatedSplTokenInfo =
                    view.findViewById(R.id.tv_approve_dialog_additional_details);
            associatedSplTokenInfo.setVisibility(View.VISIBLE);
            Spanned associatedSPLTokenAccountInfo =
                    Utils.createSpanForSurroundedPhrase(getContext(),
                            R.string.brave_wallet_confirm_transaction_account_creation_fee, (v) -> {
                                TabUtils.openUrlInNewTab(false, Utils.BRAVE_SUPPORT_URL);
                                TabUtils.bringChromeTabbedActivityToTheTop(getActivity());
                            });
            associatedSplTokenInfo.setMovementMethod(LinkMovementMethod.getInstance());
            associatedSplTokenInfo.setText(associatedSPLTokenAccountInfo);
        }
        mCoinType = TransactionUtils.getCoinFromTxDataUnion(mTxInfo.txDataUnion);
        jsonRpcService.getNetwork(mCoinType, selectedNetwork -> {
            networkName.setText(selectedNetwork.chainName);
            keyringService.getKeyringInfo(Utils.getKeyringForCoinType(mCoinType), keyringInfo -> {
                final AccountInfo[] accounts = keyringInfo.accountInfos;
                // First fill in data that does not require remote queries
                TokenUtils.getAllTokensFiltered(getBraveWalletService(), getBlockchainRegistry(),
                        selectedNetwork, selectedNetwork.coin, TokenUtils.TokenType.ALL,
                        tokenList -> {
                            SolanaTransactionsGasHelper solanaTransactionsGasHelper =
                                    new SolanaTransactionsGasHelper(
                                            (BraveWalletBaseActivity) getActivity(),
                                            new TransactionInfo[] {mTxInfo});
                            solanaTransactionsGasHelper.maybeGetSolanaGasEstimations(() -> {
                                HashMap<String, Long> perTxFee =
                                        solanaTransactionsGasHelper.getPerTxFee();
                                if (perTxFee.get(mTxInfo.id) != null) {
                                    mSolanaEstimatedTxFee = perTxFee.get(mTxInfo.id);
                                }
                                ParsedTransaction parsedTx = fillAssetDependentControls(view,
                                        selectedNetwork, accounts, new HashMap<String, Double>(),
                                        tokenList, new HashMap<String, Double>(),
                                        new HashMap<String, HashMap<String, Double>>(),
                                        mSolanaEstimatedTxFee);

                                // Get tokens involved in this transaction
                                List<BlockchainToken> tokens = new ArrayList<>();
                                tokens.add(Utils.makeNetworkAsset(
                                        selectedNetwork)); // Always add native asset
                                if (parsedTx.getIsSwap()) {
                                    tokens.add(parsedTx.getSellToken());
                                    tokens.add(parsedTx.getBuyToken());
                                } else if (parsedTx.getToken() != null)
                                    tokens.add(parsedTx.getToken());
                                BlockchainToken[] filterByTokens =
                                        tokens.toArray(new BlockchainToken[0]);

                                Utils.getTxExtraInfo((BraveWalletBaseActivity) getActivity(),
                                        selectedNetwork, accounts, filterByTokens, false,
                                        (assetPrices, fullTokenList, nativeAssetsBalances,
                                                blockchainTokensBalances) -> {
                                            fillAssetDependentControls(view, selectedNetwork,
                                                    accounts, assetPrices, fullTokenList,
                                                    nativeAssetsBalances, blockchainTokensBalances,
                                                    mSolanaEstimatedTxFee);
                                        });
                            });
                        });
            });
        });
        ImageView icon = (ImageView) view.findViewById(R.id.account_picture);
        Utils.setBlockiesBitmapResource(mExecutor, mHandler, icon, mTxInfo.fromAddress, true);
        Button reject = view.findViewById(R.id.reject);
        reject.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                rejectTransaction(true);
            }
        });
        Button approve = view.findViewById(R.id.approve);
        approve.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                approveTransaction();
            }
        });
        if (mTransactionInfos.size() > 1) {
            // TODO: next button is not functional. Update next button text based on position in
            //  mTransactionInfos list.
            //  Refactor Approve pendingTxHelper code to update the mSelectedPendingRequest Tx
            //  with the next Transaction.
            Button next = view.findViewById(R.id.btn_next_tx);
            next.setVisibility(View.VISIBLE);
            next.setOnClickListener(v -> {
                if (mTransactionConfirmationListener != null) {
                    mTransactionConfirmationListener.onNextTransaction();
                }
            });
            mRejectAllTx = view.findViewById(R.id.btn_reject_transactions);
            mRejectAllTx.setVisibility(View.VISIBLE);
            mRejectAllTx.setOnClickListener(v -> {
                if (mTransactionConfirmationListener != null) {
                    mTransactionConfirmationListener.onRejectAllTransactions();
                    dismiss();
                }
            });
            refreshListContentUi();
        }

        if (mTxInfo.originInfo != null && URLUtil.isValidUrl(mTxInfo.originInfo.originSpec)) {
            TextView domain = view.findViewById(R.id.domain);
            domain.setVisibility(View.VISIBLE);
            domain.setText(Utils.geteTLD(
                    new GURL(mTxInfo.originInfo.originSpec), mTxInfo.originInfo.eTldPlusOne));
        }
    }

    public void setTxList(List<TransactionInfo> transactionInfos) {
        mTransactionInfos = transactionInfos;
        if (isVisible()) {
            refreshListContentUi();
        }
    }

    private void refreshListContentUi() {
        mRejectAllTx.setText(getString(
                R.string.brave_wallet_queue_reject_all, String.valueOf(mTransactionInfos.size())));
    }

    private ParsedTransaction fillAssetDependentControls(View view, NetworkInfo selectedNetwork,
            AccountInfo[] accounts, HashMap<String, Double> assetPrices,
            BlockchainToken[] fullTokenList, HashMap<String, Double> nativeAssetsBalances,
            HashMap<String, HashMap<String, Double>> blockchainTokensBalances,
            long solanaEstimatedTxFee) {
        ParsedTransaction parsedTx = ParsedTransaction.parseTransaction(mTxInfo, selectedNetwork,
                accounts, assetPrices, solanaEstimatedTxFee, fullTokenList, nativeAssetsBalances,
                blockchainTokensBalances);
        TextView txType = view.findViewById(R.id.tx_type);
        if (parsedTx.getType() == TransactionType.ERC20_APPROVE) {
            txType.setText(String.format(
                    getResources().getString(R.string.activate_erc20), parsedTx.getSymbol()));
        } else if (parsedTx.getIsSwap()) {
            txType.setText(getResources().getString(R.string.swap));
        } else {
            txType.setText(getResources().getString(R.string.send));
        }

        String amountText =
                String.format(getResources().getString(R.string.crypto_wallet_amount_asset),
                        parsedTx.formatValueToDisplay(), parsedTx.getSymbol());
        TextView fromTo = view.findViewById(R.id.from_to);
        fromTo.setText(String.format(getResources().getString(R.string.crypto_wallet_from_to),
                mAccountName, parsedTx.getSender(), parsedTx.getRecipient()));
        TextView amountAsset = view.findViewById(R.id.amount_asset);
        TextView amountFiat = view.findViewById(R.id.amount_fiat);
        amountFiat.setText(
                String.format(getResources().getString(R.string.crypto_wallet_amount_fiat),
                        String.format(Locale.getDefault(), "%.2f", parsedTx.getFiatTotal())));
        if (mTxInfo.txType == TransactionType.ERC721_TRANSFER_FROM
                || mTxInfo.txType == TransactionType.ERC721_SAFE_TRANSFER_FROM) {
            amountText = Utils.tokenToString(parsedTx.getErc721BlockchainToken());
            amountFiat.setVisibility(View.GONE); // Display NFT values in the future
        }
        amountAsset.setText(amountText);
        setupPager(view, selectedNetwork, accounts, assetPrices, fullTokenList,
                nativeAssetsBalances, blockchainTokensBalances);
        return parsedTx;
    }

    private void setupPager(View view, NetworkInfo selectedNetwork, AccountInfo[] accounts,
            HashMap<String, Double> assetPrices, BlockchainToken[] fullTokenList,
            HashMap<String, Double> nativeAssetsBalances,
            HashMap<String, HashMap<String, Double>> blockchainTokensBalances) {
        ViewPager viewPager = view.findViewById(R.id.navigation_view_pager);
        ApproveTxFragmentPageAdapter adapter = new ApproveTxFragmentPageAdapter(
                getChildFragmentManager(), mTxInfo, selectedNetwork, accounts, assetPrices,
                fullTokenList, nativeAssetsBalances, blockchainTokensBalances, getActivity(),
                mTransactionConfirmationListener == null, mSolanaEstimatedTxFee);
        viewPager.setAdapter(adapter);
        viewPager.setOffscreenPageLimit(adapter.getCount() - 1);
        TabLayout tabLayout = view.findViewById(R.id.tabs);
        tabLayout.setupWithViewPager(viewPager);
    }

    private void rejectTransaction(boolean dismiss) {
        TxService txService = getTxService();
        if (txService == null) {
            return;
        }
        txService.rejectTransaction(mCoinType, mTxInfo.id, success -> {
            assert success : "tx is not rejected";
            if (!success || !dismiss) {
                return;
            }
            mRejected = true;
            if (mTransactionConfirmationListener != null) {
                mTransactionConfirmationListener.onRejectTransaction();
            }
            dismiss();
        });
    }

    private void approveTransaction() {
        TxService txService = getTxService();
        if (txService == null) {
            return;
        }
        txService.approveTransaction(mCoinType, mTxInfo.id, (success, error, errorMessage) -> {
            assert success : "tx is not approved";
            if (!success) {
                // error.getProviderError() seems to be cause an assertion crash if
                // there is no error
                Utils.warnWhenError(ApproveTxBottomSheetDialogFragment.TAG_FRAGMENT,
                        "approveTransaction", error.getProviderError(), errorMessage);
                return;
            }
            mApproved = true;
            if (mTransactionConfirmationListener != null) {
                mTransactionConfirmationListener.onApproveTransaction();
            }
            dismiss();
        });
    }

    @Override
    public void onCancel(@NonNull DialogInterface dialog) {
        super.onCancel(dialog);
        if (mTransactionConfirmationListener != null) {
            mTransactionConfirmationListener.onCancel();
        }
    }
}
