// Trezor library is loaded inside the chrome-untrusted webui page
// and communication is going through posting messages between parent window
// and frame window. This class handles low level messages transport to add,
// remove callbacks and allows to process messages for childrens.
export abstract class MessagingTransport {
  constructor () {
    this.handlers = new Map<string, Function>()
  }

  protected handlers: Map<string, Function>
    addCommandHandler = (id: string, listener: Function): boolean => {
    if (!this.handlers.size) {
      this.addWindowMessageListener()
      this.handlers.clear()
    }
    if (this.handlers.has(id)) {
      return false
    }
    this.handlers.set(id, listener)
    return true
  }

  protected removeCommandHandler = (id: string) => {
    if (!this.handlers.has(id)) {
      return false
    }
    this.handlers.delete(id)
    if (!this.handlers.size) {
      this.removeWindowMessageListener()
    }
    return true
  }

  protected abstract onMessageReceived (event: MessageEvent): unknown

  private readonly addWindowMessageListener = () => {
    window.addEventListener('message', this.onMessageReceived)
  }

  private readonly removeWindowMessageListener = () => {
    // window.removeEventListener('message', this.onMessageReceived)
  }
}

