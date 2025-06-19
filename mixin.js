(() => {
  const textDecoder = new TextDecoder()
  const memoryView = new DataView(HEAPU8.buffer)
  Module.MsgType = {
    CONTROL_VALUE: 0,
    VALUE: 1
  }
  Module.MsgSubType = {
    // value
    NEG: 0x00,
    SET_SEC_MODE: 0x01,
    SET_SSID: 0x02,
    SET_PWD: 0x03,
    CUSTOM_DATA: 0x13,
    WIFI_LIST_NEG: 0x11,
    WIFI_STATUS: 0x0F,
    ERROR: 0x12,
    // control value
    END: 0x03,
    WIFI_NEG: 0x09,
  }
  Module.BlufiError = class BlufiError {
    constructor(code) {
      this.code = code
    }
    toString () {
      return `Blufi error(0x${this.code.toString(16)})`
    }
  }
  Module.parseWifi = (bytes) => {
    const list = []
    let i = 0;
    while (i < bytes.length) {
      const len = bytes[i]
      i += 1
      const rssi = bytes[i] - 0xFF
      const ssid = textDecoder.decode(bytes.slice(i + 1, i + 1 + len - 1))
      i += len
      list.push({
        rssi,
        ssid
      })
    }
    return list
  }
  // 调用函数时，将js中的字节拷贝到wasm中，调用完后自动释放
  function safeBytesCall (func, ...bytesArray) {
    const args = bytesArray.map(bytes => {
      const ptr = Module._malloc(bytes.length)
      HEAPU8.subarray(ptr, ptr + bytes.length).set(bytes)
      return { ptr, length: bytes.length }
    })
    try {
      return func(...args)
    } finally {
      args.map(bytes => {
        Module._free(bytes.ptr)
      })
    }
  }
  function parseDatachan (ptr) {
    const list = []
    while (ptr) {
      const data = memoryView.getUint32(ptr, true);
      const len = memoryView.getUint32(ptr + 4, true)
      ptr = memoryView.getUint32(ptr + 8, true);
      list.push(HEAPU8.subarray(data, data + len))
    }
    return list
  }
  const registry = new FinalizationRegistry((core) => {
    core.delete()
  })
  Module.BlufiCore = function (mtu, callback) {
    const core = new Module.BlufiCoreInternal(mtu, callback)
    registry.register(this, core)
    this.negotiateKey = () => {
      return parseDatachan(core.negotiateKeyInternal())
    }
    this.connectWifi = (ssid, pass) => {
      return parseDatachan(core.connectWifiInternal(ssid, pass))
    }
    this.scanWifi = () => {
      return parseDatachan(core.scanWifiInternal())
    }
    this.custom = (bytes) => {
      return parseDatachan(safeBytesCall((bytes) => {
        return core.customInternal(bytes.ptr, bytes.length)
      }, bytes))
    }
    this.onReceiveData = (bytes) => {
      return safeBytesCall((bytes) => {
        return core.onReceiveDataInternal(bytes.ptr, bytes.length)
      }, bytes)
    }
  }
})();
