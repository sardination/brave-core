import * as React from 'react'

export default function () {
  return (
    <div>
      <h1>Authorize Hardware Device:</h1>
      <iframe src="chrome-untrusted://ledger-bridge" allow="hid"/>
    </div>
  )
}
