export function unbiasedRandom (min: number, max: number) {
    const range = max - min + 1
    const bytesNeeded = Math.ceil(Math.log2(range) / 8)
    const cutoff = Math.floor((256 ** bytesNeeded) / range) * range
    const bytes = new Uint8Array(bytesNeeded)
    let value
    do {
        window.crypto.getRandomValues(bytes)
        value = bytes.reduce((acc, x, n) => acc + x * 256 ** n, 0)
    } while (value >= cutoff)
    return min + value % range
}

export function randomHexString (numBytes: number = 32) {
  const bytes = new Uint8Array(numBytes)
  return [...window.crypto.getRandomValues(bytes)]
    .map(e => ('0' + e.toString(16)).slice(-2)).join('')
}
