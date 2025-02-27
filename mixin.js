(() => {
  class TimeoutError extends Error { constructor() { super(); this.name = "TimeoutError"; } }
  class BlufiCoreError extends Error {
    constructor(code) {
      super();
      this.name = "BlufiCoreError";
      this.code = code;
    }
    toString () {
      return `${this.name}: 0x${this.code.toString(16)}`
    }
  }

  Module.TimeoutError = TimeoutError
  Module.BlufiCoreError = BlufiCoreError

  let processing = null

  function delay (timeout) {
    return new Promise((_, reject) => {
      setTimeout(() => {
        reject(new TimeoutError())
      }, timeout)
    })
  }

  function checkProcessing () {
    if (processing) {
      throw new Error("Some Operation Processing")
    }
  }

  function createProcessingPromise (callback) {
    return (new Promise((resolve, reject) => {
      processing = {
        reject
      }
      callback(resolve, reject)
    })).finally(() => {
      processing = null
    })
  }

  Module.BlufiCore = function () {
    const instance = new Module.BlufiCoreInternal(...arguments)
    this.onReceiveData = async function () {
      const ret = await instance.onReceiveData(...arguments)
      if (ret != 0) {
        if (processing) {
          processing.reject(new BlufiCoreError(ret))
        } else {
          console.error("Not handled core error: 0x" + ret.toString(16))
        }
      }
    }
    this.negotiateKey = function (timeout) {
      checkProcessing();
      return Promise.race([createProcessingPromise(async (resolve, reject) => {
        const ret = await instance.negotiateKeyInternal((ret) => {
          if (ret) {
            reject(new BlufiCoreError(ret))
          } else {
            resolve()
          }
        })
        if (ret) {
          reject(new BlufiCoreError(ret));
        }
      }), delay(timeout)])
    }
    this.custom = function (bytes, timeout) {
      checkProcessing();
      return Promise.race([createProcessingPromise(async (resolve, reject) => {
        const ret = await instance.customInternal(bytes, (ret) => {
          resolve(ret)
        })
        if (ret) {
          reject(new BlufiCoreError(ret))
        }
      }), delay(timeout)])
    }
    this.scanWifi = function (timeout) {
      checkProcessing();
      return Promise.race([createProcessingPromise(async (resolve, reject) => {
        const ret = await instance.scanWifiInternal((ret) => {
          const list = [];
          for (let i = 0; i < ret.size(); i++) {
            const o = ret.get(i)
            list.push({
              ssid: o.ssid,
              rssi: o.rssi
            })
          }
          resolve(list)
        })
        if (ret) {
          reject(new BlufiCoreError(ret))
        }
      }), delay(timeout)])
    }
    this.connectWifi = function (ssid, pass, timeout) {
      checkProcessing();
      return Promise.race([createProcessingPromise(async (resolve, reject) => {
        const ret = await instance.connectWifiInternal(ssid, pass, (ret) => {
          resolve(ret)
        })
        if (ret) {
          reject(new BlufiCoreError(ret))
        }
      }), delay(timeout)])
    }
  }
})();
