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

inline val getFlattenBuffer(blufi::FlattenBuffer &buffer) {
  return val(typed_memory_view(buffer.size(), buffer.data()));
}

blufi::Core *newBlufiCore(int mtu, OnMessage OnMessage) {
  return new blufi::Core(
      mtu, [=](uint8_t type, uint8_t subType, uint8_t *data, size_t size) {
        OnMessage(type, subType, typed_memory_view(size, data));
      });
}

val scanWifi(blufi::Core &core) {
  blufi::FlattenBuffer buffer;
  std::ignore = core.scanWifi(buffer);
  return getFlattenBuffer(buffer);
}

val connectWifi(blufi::Core &core, std::string ssid, std::string pass) {
  blufi::FlattenBuffer buffer;
  std::ignore = core.connectWifi(ssid, pass, buffer);
  return getFlattenBuffer(buffer);
};

val custom(blufi::Core &core, uintptr_t data, size_t size) {
  blufi::FlattenBuffer buffer;
  std::ignore = core.custom(std::span((uint8_t *)data, size), buffer);
  return getFlattenBuffer(buffer);
}

val negotiateKey(blufi::Core &core) {
  blufi::FlattenBuffer buffer;
  std::ignore = core.negotiateKey(buffer);
  return getFlattenBuffer(buffer);
}

int onReceiveData(blufi::Core &core, uintptr_t data, size_t size) {
  return core.onReceiveData(std::span((uint8_t *)data, size));
}

EMSCRIPTEN_BINDINGS(blufi) {
  register_type<OnMessage>("(type:number,subType:number,data:any)=>void");

  class_<blufi::Core>("BlufiCoreInternal")
      .constructor(&newBlufiCore, allow_raw_pointers())
      .function("onReceiveDataInternal", &onReceiveData)
      .function("negotiateKeyInternal", &negotiateKey)
      .function("scanWifiInternal", &scanWifi)
      .function("connectWifiInternal", &connectWifi)
      .function("customInternal", &custom);
}