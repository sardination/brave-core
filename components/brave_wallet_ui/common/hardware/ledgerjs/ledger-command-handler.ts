// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import { LedgerFrameCommand, LedgerCommand } from './ledger-messages'
import { MessagingTransport } from '../messaging-transport'

// Handles commands forwarding to the Ledger library inside the iframe.
export class LedgerCommandHandler extends MessagingTransport {
  sendAuthorizationResultToParent = () => {
    window.parent.postMessage('authorize success', window.parent.origin)
  }

  protected onMessageReceived = async (event: MessageEvent) => {
    if (event.origin !== event.data.origin || event.type !== 'message' || !event.source) {
      return
    }
    const message = event.data as LedgerFrameCommand
    if (!message || !this.handlers.has(message.command)) {
      return
    }
    const callback = this.handlers.get(message.command) as Function
    const response = await callback.call(this, event.data)
    const target = event.source as Window
    target.postMessage(response, response.origin)
  }
}

let handler: LedgerCommandHandler

export function addLedgerCommandHandler (command: LedgerCommand, listener: Function): Boolean {
  if (!handler) {
    handler = new LedgerCommandHandler()
  }
  return handler.addCommandHandler(command, listener)
}

