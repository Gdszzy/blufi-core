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

DataChan *datachan = NULL;

void initDataChan() {
  if(datachan != NULL) {
    freeDataChan(datachan);
  }
  datachan = newDataChan();
}

blufi::Core *newBlufiCore(int mtu, OnMessage OnMessage) {
  return new blufi::Core(
      mtu, [=](uint8_t type, uint8_t subType, uint8_t *data, size_t size) {
        OnMessage(type, subType, typed_memory_view(size, data));
      });
}

uintptr_t scanWifi(blufi::Core &core) {
  initDataChan();
  std::ignore = core.scanWifi(datachan);
  return (uintptr_t)datachan;
}

uintptr_t connectWifi(blufi::Core &core, std::string ssid, std::string pass) {
  initDataChan();
  std::ignore = core.connectWifi(datachan, ssid, pass);
  return (uintptr_t)datachan;
};

uintptr_t custom(blufi::Core &core, uintptr_t data, size_t size) {
  initDataChan();
  std::ignore = core.custom(datachan, std::span((uint8_t *)data, size));
  return (uintptr_t)datachan;
}

uintptr_t negotiateKey(blufi::Core &core) {
  initDataChan();
  std::ignore = core.negotiateKey(datachan);
  return (uintptr_t)datachan;
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