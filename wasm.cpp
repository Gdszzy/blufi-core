#include "blufi.h"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/promise.h>
#include <emscripten/val.h>

using namespace emscripten;

EMSCRIPTEN_DECLARE_VAL_TYPE(OnMessage);

// void consoleLog(std::string msg) {
//   auto console = val::global("console");
//   console.call<void>("log", msg);
// }

blufi::SendData sendData;

val vector2val(std::vector<std::vector<uint8_t>> &data) {
  val arr = val::array();
  for(int i = 0; i < data.size(); i++) {
    std::vector<uint8_t> &slice = data[i];
    arr.set(i, typed_memory_view(slice.size(), slice.data()));
  }
  return arr;
}

blufi::Core *newBlufiCore(int mtu, OnMessage OnMessage) {
  return new blufi::Core(
      mtu, [=](uint8_t type, uint8_t subType, std::vector<uint8_t> *data) {
        OnMessage(type, subType, typed_memory_view(data->size(), data->data()));
      });
}

val scanWifi(blufi::Core &core) {
  sendData.clear();
  std::ignore = core.scanWifi(sendData);
  return vector2val(sendData);
}

val connectWifi(blufi::Core &core, std::string ssid, std::string pass) {
  sendData.clear();
  std::ignore = core.connectWifi(sendData, ssid, pass);
  return vector2val(sendData);
};

val custom(blufi::Core &core, uintptr_t data, size_t size) {
  sendData.clear();
  std::ignore = core.custom(sendData, std::span((uint8_t *)data, size));
  return vector2val(sendData);
}

val negotiateKey(blufi::Core &core) {
  sendData.clear();
  std::ignore = core.negotiateKey(sendData);
  return vector2val(sendData);
}

int onReceiveData(blufi::Core &core, uintptr_t data, size_t size) {
  return core.onReceiveData(std::span((uint8_t *)data, size));
}

EMSCRIPTEN_BINDINGS(blufi) {
  register_type<OnMessage>("(type:number,subType:number,data:any)=>void");

  class_<blufi::Core>("BlufiCore")
      .constructor(&newBlufiCore, allow_raw_pointers())
      .function("onReceiveDataInternal", &onReceiveData)
      .function("negotiateKey", &negotiateKey)
      .function("scanWifi", &scanWifi)
      .function("connectWifi", &connectWifi)
      .function("customInternal", &custom);
}