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

std::vector<uint8_t> val2vector(val list) {
  std::vector<uint8_t> arr;
  size_t len = list["length"].as<size_t>();
  for(int i = 0; i < len; i++) {
    arr.push_back(list[i].as<uint8_t>());
  }
  return arr;
}

val vector2val(std::vector<std::vector<uint8_t>> &data) {
  val arr = val::array();
  for(int i = 0; i < data.size(); i++) {
    std::vector<uint8_t> &slice = data[i];
    arr.set(i, typed_memory_view(slice.size(), slice.data()));
  }
  return arr;
}

blufi::Core *newBlufiCore(int mtu, OnMessage OnMessage) {
  return new blufi::Core(mtu, [=](uint8_t type, uint8_t subType, void *data) {
    // consoleLog("on msg");
    if(type == msg::Type::VALUE) {
      if(subType == msg::SubType::WIFI_LIST_NEG) {
        // 只有wifi列表需要单独处理
        std::vector<blufi::Wifi> *list = (std::vector<blufi::Wifi> *)data;
        val wifiList = val::array();
        for(int i = 0; i < list->size(); i++) {
          blufi::Wifi &wifi = list->at(i);
          val wifiInfo = val::object();
          wifiInfo.set("ssid", wifi.ssid);
          wifiInfo.set("rssi", wifi.rssi);
          wifiList.set(i, wifiInfo);
        }
        // 回调
        OnMessage(type, subType, wifiList);
        return;
      }
    }
    // 其他都当成字节数组处理
    std::vector<uint8_t> *bytes = (std::vector<uint8_t> *)data;
    OnMessage(type, subType, typed_memory_view(bytes->size(), bytes->data()));
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

val custom(blufi::Core &core, val data) {
  sendData.clear();
  std::ignore = core.custom(sendData, val2vector(data));
  return vector2val(sendData);
}

val negotiateKey(blufi::Core &core) {
  sendData.clear();
  std::ignore = core.negotiateKey(sendData);
  return vector2val(sendData);
}

int onReceiveData(blufi::Core &core, val bytes) {
  auto buf = val2vector(bytes);
  return core.onReceiveData(std::span(buf));
}

EMSCRIPTEN_BINDINGS(blufi) {
  register_type<OnMessage>("(type:number,subType:number,data:any)=>void");

  class_<blufi::Core>("BlufiCore")
      .constructor(&newBlufiCore, allow_raw_pointers())
      .function("onReceiveData", &onReceiveData, async())
      .function("negotiateKey", &negotiateKey, async())
      .function("scanWifi", &scanWifi, async())
      .function("connectWifi", &connectWifi, async())
      .function("custom", &custom, async());
}