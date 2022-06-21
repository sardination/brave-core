// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
import { loadTimeData } from '../../../../common/loadTimeData'
import { Unsuccessful, EthereumSignTransaction, CommonParams, Success } from 'trezor-connect'
import { HDNodeResponse } from 'trezor-connect/lib/typescript'
import { EthereumSignedTx, EthereumSignMessage, EthereumSignTypedHash } from 'trezor-connect/lib/typescript/networks/ethereum'
import { MessageSignature } from 'trezor-connect/lib/typescript/trezor/protobuf'
export const kTrezorBridgeUrl = loadTimeData.getString('braveWalletTrezorBridgeUrl')

export enum TrezorCommand {
  Unlock = 'trezor-unlock',
  GetAccounts = 'trezor-get-accounts',
  SignTransaction = 'trezor-sign-transaction',
  SignMessage = 'trezor-sign-message',
  SignTypedMessage = 'trezor-sign-typed-message'
}

export enum TrezorErrorsCodes {
  BridgeNotReady = 0,
  CommandInProgress = 1
  // AuthorizationNeeded = 2
}

export type CommandMessage = {
  command: TrezorCommand
  id: string
  origin: string
}
export type TrezorAccountPath = {
  path: string
}
export type TrezorAccount = {
  publicKey: string
  serializedPath: string
  fingerprint: number
}
export type TrezorError = {
  error: string
  code?: string | number
}

// Unlock command
export type UnlockResponse = Unsuccessful | {
  success: boolean
}
export type UnlockResponsePayload = CommandMessage & {
  payload: UnlockResponse
}
export type UnlockCommand = CommandMessage & {
  command: TrezorCommand.Unlock
}

// GetAccounts command
export type TrezorGetAccountsResponse = Unsuccessful | Success<HDNodeResponse[]>
export type GetAccountsResponsePayload = CommandMessage & {
  payload: TrezorGetAccountsResponse
}
export type GetAccountsCommand = CommandMessage & {
  command: TrezorCommand.GetAccounts
  paths: TrezorAccountPath[]
}

// SignTransaction command
export type SignTransactionCommandPayload = CommonParams & EthereumSignTransaction
export type SignTransactionCommand = CommandMessage & {
  command: TrezorCommand.SignTransaction
  payload: SignTransactionCommandPayload
}
export type SignTransactionResponse = Unsuccessful | Success<EthereumSignedTx>
export type SignTransactionResponsePayload = CommandMessage & {
  payload: SignTransactionResponse
}

// SignMessage command
export type SignMessageCommandPayload = CommonParams & EthereumSignMessage
export type SignMessageCommand = CommandMessage & {
  command: TrezorCommand.SignMessage
  payload: SignMessageCommandPayload
}
export type SignMessageResponse = Unsuccessful | Success<MessageSignature>
export type SignMessageResponsePayload = CommandMessage & {
  payload: SignMessageResponse
}

// SignTypedMessage command
export type SignTypedMessageCommandPayload = CommonParams & EthereumSignTypedHash
export type SignTypedMessageCommand = CommandMessage & {
  command: TrezorCommand.SignTypedMessage
  payload: SignTypedMessageCommandPayload
}
export type SignTypedMessageResponse = Unsuccessful | Success<MessageSignature>
export type SignTypedMessageResponsePayload = CommandMessage & {
  payload: SignTypedMessageResponse
}

export type TrezorFrameCommand = GetAccountsCommand | UnlockCommand | SignTransactionCommand | SignMessageCommand | SignTypedMessageCommand
export type TrezorFrameResponse = UnlockResponsePayload | GetAccountsResponsePayload | SignTransactionResponsePayload | SignMessageResponsePayload
