/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { LEDGER_HARDWARE_VENDOR } from 'gen/brave/components/brave_wallet/common/brave_wallet.mojom.m.js'
import { BraveWallet } from '../../../constants/types'
import { getLocale } from '../../../../common/locale'
import { LedgerSolanaKeyring } from '../interfaces'
import { HardwareVendor } from '../../api/hardware_keyrings'
import {
  GetAccountsHardwareOperationResult,
  HardwareOperationResult,
  SignHardwareMessageOperationResult,
  SignHardwareTransactionOperationResult
} from '../types'
import {
  LedgerCommand,
  UnlockResponsePayload,
  LedgerFrameCommand,
  GetAccountResponse,
  GetAccountResponsePayload,
  SignTransactionResponse,
  SignTransactionResponsePayload,
  LedgerErrorsCodes,
} from './ledger-messages'
import {
  sendLedgerCommand,
  closeLedgerBridge
} from './ledger-bridge-transport'
import { hardwareDeviceIdFromAddress } from '../hardwareDeviceIdFromAddress'

export default class SolanaLedgerKeyring implements LedgerSolanaKeyring {
  private deviceId: string
  private unlocked: boolean = false
  private onAuthorize?: Function

  constructor(onAuthorize?: Function) {
    this.onAuthorize = onAuthorize
  }

  coin = (): BraveWallet.CoinType => {
    return BraveWallet.CoinType.SOL
  }

  type = (): HardwareVendor => {
    return LEDGER_HARDWARE_VENDOR
  }

  cancelOperation = async () => {
    closeLedgerBridge()
  }

  unlock = async (): Promise<HardwareOperationResult> => {
    if (this.isUnlocked()) {
      return { success: true }
    }

    const data = await this.sendLedgerCommand<UnlockResponsePayload>({
      id: LedgerCommand.Unlock,
      origin: window.origin,
      command: LedgerCommand.Unlock
    })
    if (data === LedgerErrorsCodes.BridgeNotReady ||
        data === LedgerErrorsCodes.CommandInProgress) {
      return this.createErrorFromCode(data)
    }

    this.unlocked = data.payload.success
    if (!data.payload.success) {
      return data.payload
      // return { success: false, error: 'unauthorized', code: 'unauthorized'}
      // return { success: false, error: getLocale('braveWalletUnlockError') }
    }

    return { success: this.unlocked }
  }

  isUnlocked = (): boolean => {
    return this.unlocked
  }

  getAccounts = async (from: number, to: number): Promise<GetAccountsHardwareOperationResult> => {
    if (!this.isUnlocked()) {
      const unlocked = await this.unlock()
      if (!unlocked.success) {
        return unlocked
      }
    }
    from = (from >= 0) ? from : 0
    const paths = []
    const addZeroPath = (from > 0 || to < 0)
    if (addZeroPath) {
      // Add zero address to calculate device id.
      paths.push(this.getPathForIndex(0))
    }
    for (let i = from; i <= to; i++) {
      paths.push(this.getPathForIndex(i))
    }
    return this.getAccountsFromDevice(paths, addZeroPath)
  }

  signPersonalMessage (path: string, address: string, message: string): Promise<SignHardwareMessageOperationResult> {
    throw new Error('Method not implemented.')
  }

  signTransaction = async (path: string, rawTxBytes: Buffer): Promise<SignHardwareTransactionOperationResult> => {
    if (!this.isUnlocked()) {
      const unlocked = await this.unlock()
      if (!unlocked.success) {
        return unlocked
      }
    }
    const data = await this.sendLedgerCommand<SignTransactionResponsePayload>({
      command: LedgerCommand.SignTransaction,
      id: LedgerCommand.SignTransaction,
      path: path,
      rawTxBytes: rawTxBytes,
      origin: window.origin
    })
    if (data === LedgerErrorsCodes.BridgeNotReady ||
        data === LedgerErrorsCodes.CommandInProgress) {
      return this.createErrorFromCode(data)
    }
    const response: SignTransactionResponse = data.payload
    return { success: true, payload: response.signature }
  }

  private async sendLedgerCommand<T> (command: LedgerFrameCommand): Promise<T | LedgerErrorsCodes> {
    return sendLedgerCommand<T>(command, this.onAuthorize)
  }

  private readonly getAccountsFromDevice = async (paths: string[], skipZeroPath: boolean): Promise<GetAccountsHardwareOperationResult> => {
    let accounts = []
    const zeroPath = this.getPathForIndex(0)
    for (const path of paths) {
      const data = await this.sendLedgerCommand<GetAccountResponsePayload>({
        command: LedgerCommand.GetAccount,
        id: LedgerCommand.GetAccount,
        path: path,
        origin: window.origin
      })

      if (data === LedgerErrorsCodes.BridgeNotReady ||
          data === LedgerErrorsCodes.CommandInProgress) {
        return this.createErrorFromCode(data)
      }
      const response: GetAccountResponse = data.payload

      if (path === zeroPath) {
        this.deviceId = await hardwareDeviceIdFromAddress(response.address)
        if (skipZeroPath) {
          // If requested addresses do not have zero indexed adress we add it
          // intentionally to calculate device id and should not add it to
          // returned accounts
          continue
        }
      }

      accounts.push({
        address: '',
        addressBytes: response.address,
        derivationPath: path,
        name: this.type(),
        hardwareVendor: this.type(),
        deviceId: this.deviceId,
        coin: this.coin()
      })
    }
    return { success: true, payload: accounts }
  }

  private readonly createErrorFromCode = (code: LedgerErrorsCodes): HardwareOperationResult => {
    switch (code) {
      case LedgerErrorsCodes.BridgeNotReady:
        return { success: false, error: getLocale('braveWalletBridgeNotReady'), code: code }
      case LedgerErrorsCodes.CommandInProgress:
        return { success: false, error: getLocale('braveWalletBridgeCommandInProgress'), code: code }
    }
  }
  private readonly getPathForIndex = (index: number): string => {
    return `44'/501'/${index}'/0'`
  }
}
