(() => {
  const textDecoder = new TextDecoder()
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
  function postRun () {
    Module.BlufiCore.prototype.custom = function (bytes) {
      return safeBytesCall((bytes) => {
        return this.customInternal(bytes.ptr, bytes.length)
      }, bytes)
    }
    Module.BlufiCore.prototype.onReceiveData = function (bytes) {
      return safeBytesCall((bytes) => {
        return this.onReceiveDataInternal(bytes.ptr, bytes.length)
      }, bytes)
    }
  }
  if (runtimeInitialized) {
    postRun();
  } else {
    addOnPostRun(postRun)
  }
})();
