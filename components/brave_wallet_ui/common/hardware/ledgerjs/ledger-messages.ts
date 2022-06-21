/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { loadTimeData } from '../../../../common/loadTimeData'
export const kLedgerBridgeUrl = loadTimeData.getString('braveWalletLedgerBridgeUrl')

export enum LedgerCommand {
  Unlock = 'ledger-unlock',
  GetAccount = 'ledger-get-accounts',
  SignTransaction = 'ledger-sign-transaction'
}

// Could be abstracted / shared
export enum LedgerErrorsCodes {
  BridgeNotReady = 0,
  CommandInProgress = 1
}

export type CommandMessage = {
  command: LedgerCommand
  id: string
  origin: string
}

// Unlock command
// export type UnlockResponse = Unsuccessful | {
export type UnlockResponse = {
  success: boolean
}
export type UnlockResponsePayload = CommandMessage & {
  payload: UnlockResponse
}
export type UnlockCommand = CommandMessage & {
  command: LedgerCommand.Unlock
}

// GetAccounts command
export type GetAccountResponse = {
  address: Buffer
}
export type GetAccountResponsePayload = CommandMessage & {
  payload: GetAccountResponse
}
export type GetAccountCommand = CommandMessage & {
  command: LedgerCommand.GetAccount
  path: string
}

// SignTransaction command
// export type SignTransactionResponse = Unsuccessful | Success<EthereumSignedTx>
export type SignTransactionResponse = {
  signature: Buffer
}
export type SignTransactionResponsePayload = CommandMessage & {
  payload: SignTransactionResponse
}
export type SignTransactionCommand = CommandMessage & {
  command: LedgerCommand.SignTransaction
  path: string,
  rawTxBytes: Buffer
}

export type LedgerFrameCommand = UnlockCommand | GetAccountCommand | SignTransactionCommand;
export type LedgerFrameResponse = UnlockResponsePayload | GetAccountResponsePayload | SignTransactionResponsePayload; 
