#include "blufi.h"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/promise.h>
#include <emscripten/val.h>

using namespace emscripten;

EMSCRIPTEN_DECLARE_VAL_TYPE(OnSendData);
EMSCRIPTEN_DECLARE_VAL_TYPE(NegotiateResult);
EMSCRIPTEN_DECLARE_VAL_TYPE(ScanWifiResult);
EMSCRIPTEN_DECLARE_VAL_TYPE(BytesResult);

void consoleLog(std::string msg) {
  auto console = val::global("console");
  console.call<void>("log", msg);
}

std::vector<uint8_t> val2vector(val list) {
  std::vector<uint8_t> arr;
  size_t len = list["length"].as<size_t>();
  for(int i = 0; i < len; i++) {
    arr.push_back(list[i].as<uint8_t>());
  }
  return arr;
}

blufi::Core *newBlufiCore(int mtu, OnSendData onSendData) {
  consoleLog("exec");
  return new blufi::Core(mtu, [=](std::span<uint8_t> data) {
    auto jsBytes = val(typed_memory_view(data.size(), data.data()));
    consoleLog("before send");
    auto ret = onSendData(jsBytes).await();
    consoleLog("end");
    // consoleLog("raw ret" + std::to_string(ret));
    return 0;
  });
}

int scanWifi(blufi::Core &core, ScanWifiResult onResult) {
  return core.scanWifi(
      [=](std::vector<blufi::Wifi> result) { onResult(result); });
}

int connectWifi(blufi::Core &core, std::string ssid, std::string pass,
                BytesResult onResult) {
  return core.connectWifi(ssid, pass, [=](const std::span<uint8_t> &result) {
    onResult(val(typed_memory_view(result.size(), result.data())));
  });
};

int custom(blufi::Core &core, val data, BytesResult onResult) {
  return core.custom(val2vector(data), [=](const std::span<uint8_t> &result) {
    onResult(val(typed_memory_view(result.size(), result.data())));
  });
}

int negotiateKey(blufi::Core &core, NegotiateResult onResult) {
  return core.negotiateKey([=](int result) { onResult(result); });
}

int onReceiveData(blufi::Core &core, val bytes) {
  auto buf = val2vector(bytes);
  return core.onReceiveData(std::span(buf));
}

EMSCRIPTEN_BINDINGS(blufi) {
  register_type<OnSendData>("(bytes: Uint8Array) => number");
  register_type<NegotiateResult>("(ret: number)=>void");
  register_type<ScanWifiResult>("(wifiList:Array<Wifi>)=>void");
  register_type<BytesResult>("(bytes:Uint8Array)=>void");
  class_<blufi::Core>("BlufiCore")
      .constructor(&newBlufiCore, allow_raw_pointers())
      .function("onReceiveData", &onReceiveData)
      .function("negotiateKey", &negotiateKey)
      .function("scanWifi", &scanWifi)
      .function("connectWifi", &connectWifi)
      .function("custom", &custom);
  class_<blufi::Wifi>("Wifi")
      .property("ssid", &blufi::Wifi::ssid)
      .property("rssi", &blufi::Wifi::rssi);
}