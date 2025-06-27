## blufi-core

This library is mainly to simplify the use of blufi.

The core written by C++ and can be compiled to WebAssembly and JNI library.

Not it support:

- web (WebAssembly+JavaScript)
- Android (JNI)

### Usage (WebAssembly+JavaScript)

```js
import blufiLoader from './blufi.js'
import blufiWasmUrl from './blufi.wasm?url'

// Loading wasm module
const blufiModule = (blufiModule = await blufiLoader({
  // Using locateFile or any other way supported in Emscripten
  locateFile: () => blufiWasmUrl
}))

// Scanning and connect to your BLE device
connectBLEDevice()
const blufiWriteChar = getBlufiWriteChar()
const blufiNotifyChar = getBlufiReadChar()

const core = new blufiModule.BlufiCore(
  // mtu
  128,
  // onBlufiMessageCallback
  (type, subType, buffer) => {
    if (type == blufiModule.MsgType.VALUE) {
      if (subType == blufiModule.MsgSubType.NEG) {
        // Negotiate message
        // You have to send the [buffer] to [blufiWriteChar]
      }
      // Other message process
    }
  }
)

// Enable notification
blufiNotifyChar.oncharacteristicvaluechanged = (evt) => {
  const buf = new Uint8Array(evt.currentTarget.value.buffer)
  // Send all data received into core
  // When a complete message received. onBlufiMessageCallback will be called
  core.onReceiveData(buf)
}
blufiNotifyChar.startNotifications()

// Initialization completed. Start to use core
// All of these functions just help you to create the message you need send.
// Then you have to sent them by yourself.
const negotiateKeyMessage: Array<Uint8Array> = core.negotiateKey()
core.scanWifi()
core.connectWifi(ssid, password)
core.custom(new Uint8Array([0xff, 0xaa]))
```
