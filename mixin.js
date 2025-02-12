addOnPostRun(() => {
  const proto = Module.BlufiCore.prototype;
  function delay (timeout) {
    return new Promise((resolve) => {
      setTimeout(resolve, timeout)
    })
  }
  proto.negotiateKey = function (timeout) {
    return Promise.race([new Promise(async (resolve, reject) => {
      const ret = await this.negotiateKeyInternal((ret) => {
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
  proto.custom = function (bytes, timeout) {
    return Promise.race([new Promise(async (resolve, reject) => {
      const ret = await this.customInternal(bytes, (ret) => {
        resolve(ret)
      })
      if (ret) {
        reject(ret)
      }
    }), delay(timeout)])
  }
  proto.scanWifi = function (timeout) {
    return Promise.race([new Promise(async (resolve, reject) => {
      const ret = await this.scanWifiInternal((ret) => {
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
  proto.connectWifi = function (ssid, pass, timeout) {
    return Promise.race([new Promise(async (resolve, reject) => {
      const ret = await this.connectWifiInternal(ssid, pass, (ret) => {
        resolve(ret)
      })
      if (ret) {
        reject(ret)
      }
    }), delay(timeout)])
  }
});
