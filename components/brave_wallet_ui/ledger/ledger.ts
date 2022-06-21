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

import { addLedgerCommandHandler } from '../common/hardware/ledgerjs/ledger-command-handler'

const createUnlockResponse = (command: UnlockCommand, result: boolean, error?: any): UnlockResponsePayload => {
  const payload: UnlockResponse = (!result && error) ? error : { success: result }
  return { id: command.id, command: command.command, payload: payload, origin: command.origin }
}

const createGetAccountResponse = (command: GetAccountCommand, result: GetAccountResponse): GetAccountResponsePayload => {
  return { id: command.id, command: command.command, payload: result, origin: command.origin }
}

const authorizationNeeded = async (): Promise<boolean> => {
  return (await TransportWebHID.list()).length === 0
}

// Set up event listener for authorize button
let authorizeBtn
window.addEventListener('DOMContentLoaded', (event) => {
  authorizeBtn = document.getElementById('authorize')
  if (authorizeBtn) {
    authorizeBtn.addEventListener('click', () => {
      TransportWebHID.request().then((transport) => {
        app = new Sol(transport)
        window.parent.postMessage('authorize success', 'chrome://wallet')
      })
      // TODO handle authorize failure / deny
    })
  } 
})

let app: Sol

// Handlers 
addLedgerCommandHandler(LedgerCommand.Unlock, (command: UnlockCommand): Promise<UnlockResponsePayload> => {
  return new Promise(async (resolve) => {
    if ((await authorizationNeeded())) {
      const error = {
        success: false,
        error: 'unauthorized',
        code: 'unauthorized'
      }
      resolve(createUnlockResponse(command, false, error))
    }
    
    TransportWebHID.create().then((transport) => {
      app = new Sol(transport)
      resolve(createUnlockResponse(command, true))
    }).catch((error: any) => {
      resolve(createUnlockResponse(command, false, error))
    })
  })
})

addLedgerCommandHandler(LedgerCommand.GetAccount, (command: GetAccountCommand, source: Window): Promise<GetAccountResponsePayload> => {
  return new Promise(async (resolve) => {
    app.getAddress(command.path).then((result: GetAccountResponse) => {
      resolve(createGetAccountResponse(command, result))
    })
  })
})

addLedgerCommandHandler(LedgerCommand.SignTransaction, (command: SignTransactionCommand, source: Window): Promise<SignTransactionResponsePayload> => {
  return new Promise(async (resolve) => {
    app.signTransaction(command.path, Buffer.from(command.rawTxBytes)).then((result: SignTransactionResponse) => {
      resolve({ id: command.id, command: command.command, payload: result, origin: command.origin })
    })
  })
})
