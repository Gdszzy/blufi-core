(() => {
  class TimeoutError extends Error { constructor() { super(); this.name = "TimeoutError"; } }
  class SendError extends Error {
    constructor(code) {
      super()
      this.name = "SendError";
      this.code = code;
    }
    toString () {
      return `${this.name}: ${this.message} (${this.code})`
    }
  }
  class ReceiveError extends Error {
    constructor(code) {
      super(
        code == 0x10 ? "Wrong Data Length" :
          code == 0x20 ? "Invalid Sequence" :
            code == 0x30 ? "Empty Data" :
              code == 0x40 ? "No Key" :
                code == 0x50 ? "Invalid Type" :
                  code == 0x60 ? "Action Not Match" :
                    code == 0x70 ? "Remote Error" :
                      code == 0x80 ? "Not Implement" : "Unkown Error"
      );
      this.name = "ReceiveError";
      this.code = code;
    }
    toString () {
      return `${this.name}: ${this.message} (0x${this.code.toString(16)})`
    }
  }

  Module.TimeoutError = TimeoutError
  Module.ReceiveError = ReceiveError

  function delay (timeout) {
    return new Promise((_, reject) => {
      setTimeout(() => {
        reject(new TimeoutError())
      }, timeout)
    })
  }

  Module.BlufiCore = function () {
    const instance = new Module.BlufiCoreInternal(...arguments)
    this.onReceiveData = async function () {
      const ret = await instance.onReceiveData(...arguments)
      if (ret != 0) {
        throw new ReceiveError(ret);
      }
    }
    this.negotiateKey = function (timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
        const ret = await instance.negotiateKeyInternal((ret) => {
          if (ret) {
            reject(new SendError(ret))
          } else {
            resolve()
          }
        })
        if (ret) {
          reject(new SendError(ret));
        }
      }), delay(timeout)])
    }
    this.custom = function (bytes, timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
        const ret = await instance.customInternal(bytes, (ret) => {
          resolve(ret)
        })
        if (ret) {
          reject(new SendError(ret))
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
          reject(new SendError(ret))
        }
      }), delay(timeout)])
    }
    this.connectWifi = function (ssid, pass, timeout) {
      return Promise.race([new Promise(async (resolve, reject) => {
        const ret = await instance.connectWifiInternal(ssid, pass, (ret) => {
          resolve(ret)
        })
        if (ret) {
          reject(new SendError(ret))
        }
      }), delay(timeout)])
    }
  }
})();
