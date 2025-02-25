(() => {
  function delay (timeout) {
    return new Promise((resolve) => {
      setTimeout(resolve, timeout)
    })
  }
  Module.BlufiCore = function () {
    console.log(arguments)
    const instance = new Module.BlufiCoreInternal(...arguments)
    this.onReceiveData = function () {
      return instance.onReceiveData(...arguments)
    }
    this.negotiateKey = function (timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
        const ret = await instance.negotiateKeyInternal((ret) => {
          if (ret) {
            reject(ret)
          } else {
            resolve()
          }
        })
        if (ret) {
          reject(ret);
        }
      }), delay(timeout)])
    }
    this.custom = function (bytes, timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
        const ret = await instance.customInternal(bytes, (ret) => {
          resolve(ret)
        })
        if (ret) {
          reject(ret)
        }
      }), delay(timeout)])
    }
    this.scanWifi = function (timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
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
          reject(ret)
        }
      }), delay(timeout)])
    }
    this.connectWifi = function (ssid, pass, timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
        const ret = await instance.connectWifiInternal(ssid, pass, (ret) => {
          resolve(ret)
        })
        if (ret) {
          reject(ret)
        }
      }), delay(timeout)])
    }
  }
})();
