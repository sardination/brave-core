/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import TransportWebHID from '@ledgerhq/hw-transport-webhid'
import Sol from '@ledgerhq/hw-app-solana'

import {
  LedgerCommand,
  UnlockCommand,
  UnlockResponse,
  UnlockResponsePayload,
  GetAccountCommand,
  GetAccountResponse,
  GetAccountResponsePayload,
  SignTransactionCommand,
  SignTransactionResponsePayload,
  SignTransactionResponse,
} from '../common/hardware/ledgerjs/ledger-messages'

import { addTrezorCommandHandler } from '../common/hardware/trezor/trezor-command-handler'

const createUnlockResponse = (command: UnlockCommand, result: boolean, error?: any): UnlockResponsePayload => {
  const payload: UnlockResponse = (!result && error) ? error : { success: result }
  return { id: command.id, command: command.command, payload: payload, origin: command.origin }
}

const createGetAccountResponse = (command: GetAccountCommand, result: GetAccountResponse): GetAccountResponsePayload => {
  return { id: command.id, command: command.command, payload: result, origin: command.origin }
}

let app: Sol

addTrezorCommandHandler(LedgerCommand.Unlock, (command: UnlockCommand): Promise<UnlockResponsePayload> => {
  return new Promise(async (resolve) => {
    TransportWebHID.create().then((transport) => {
      app = new Sol(transport)
      resolve(createUnlockResponse(command, true))
    }).catch((error: any) => {
      resolve(createUnlockResponse(command, false, error))
    })
  })
})

addTrezorCommandHandler(LedgerCommand.GetAccount, (command: GetAccountCommand, source: Window): Promise<GetAccountResponsePayload> => {
  return new Promise(async (resolve) => {
    app.getAddress(command.path).then((result: GetAccountResponse) => {
      resolve(createGetAccountResponse(command, result))
    })
  })
})

addTrezorCommandHandler(LedgerCommand.SignTransaction, (command: SignTransactionCommand, source: Window): Promise<SignTransactionResponsePayload> => {
  return new Promise(async (resolve) => {
    app.signTransaction(command.path, Buffer.from(command.rawTxBytes)).then((result: SignTransactionResponse) => {
      resolve({ id: command.id, command: command.command, payload: result, origin: command.origin })
    })
  })
})
