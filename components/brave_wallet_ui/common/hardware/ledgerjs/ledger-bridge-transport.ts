// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import { MessagingTransport } from '../messaging-transport'

import {
  kLedgerBridgeUrl,
  LedgerErrorsCodes,
  LedgerFrameCommand
} from '../ledgerjs/ledger-messages'

// Handles sending messages to the Ledger library. It creates untrusted iframe
// with origin chrome-untrusted://trezor-bridge where Ledger library is isolated and run.
// It send commands to the iframe via postMessage and subscribes
// for responses.
export class LedgerBridgeTransport extends MessagingTransport {
  // constructor (bridgeFrameUrl: string) {
  constructor (bridgeFrameUrl: string, onAuthorize?: Function) {
    super()
    this.bridgeFrameUrl = bridgeFrameUrl
    // @ts-expect-error
    this.frameId = crypto.randomUUID()
    this.onAuthorize = onAuthorize
  }

  private onAuthorize?: Function
  private readonly frameId: string
  private readonly bridgeFrameUrl: string
  private bridge?: HTMLIFrameElement

  closeBridge = () => {
    if (!this.bridge || !this.hasBridgeCreated()) {
      return
    }
    const element = document.getElementById(this.frameId)
    element?.parentNode?.removeChild(element)
  }

  // T is response type, e.g. UnlockResponse. Resolves as `false` if transport error
  sendCommandToLedgerFrame = <T> (command: LedgerFrameCommand): Promise<T | LedgerErrorsCodes > => {
    return new Promise<T | LedgerErrorsCodes >(async (resolve) => {
      if (!this.bridge && !this.hasBridgeCreated()) {
        this.bridge = await this.createBridge()
      }
      if (!this.bridge || !this.bridge.contentWindow) {
        resolve(LedgerErrorsCodes.BridgeNotReady)
        return
      }
      if (!this.addCommandHandler(command.id, resolve)) {
        resolve(LedgerErrorsCodes.CommandInProgress)
        return
      }
      this.bridge.contentWindow.postMessage(command, this.bridgeFrameUrl)
    })
  }

  protected onMessageReceived = (event: MessageEvent) => {
    // todo pass a message with better typing
    if (event.data === 'authorize success') {
      if (this.onAuthorize) { 
        this.onAuthorize()
      }
    }

    if (event.origin !== this.getLedgerBridgeOrigin() ||
        event.type !== 'message' ||
        !this.handlers.size) {
      return
    }

    const message = event.data as LedgerFrameCommand
    if (!message || !this.handlers.has(message.id)) {
      return
    }
    const callback = this.handlers.get(message.id) as Function
    callback.call(this, message)
    this.removeCommandHandler(event.data.id)
  }

  private readonly getLedgerBridgeOrigin = () => {
    return (new URL(this.bridgeFrameUrl)).origin
  }

  private readonly createBridge = () => {
    return new Promise<HTMLIFrameElement>((resolve) => {
      let element = document.createElement('iframe')
      element.id = this.frameId
      element.src = this.bridgeFrameUrl
      element.style.display = 'none'
      if (this.bridgeFrameUrl == kLedgerBridgeUrl) {
        element.allow = 'hid'
      }
      element.onload = () => {
        resolve(element)
      }
      document.body.appendChild(element)
    })
  }

  private readonly hasBridgeCreated = (): boolean => {
    return document.getElementById(this.frameId) !== null
  }
}

let transport: LedgerBridgeTransport

export async function sendLedgerCommand<T> (command: LedgerFrameCommand, onAuthorize?: Function): Promise<T | LedgerErrorsCodes> {
  if (!transport) {
    transport = new LedgerBridgeTransport(kLedgerBridgeUrl, onAuthorize)
  }
  return transport.sendCommandToLedgerFrame<T>(command)
}

export async function closeLedgerBridge () {
  if (!transport) {
    return
  }
  return transport.closeBridge()
}
