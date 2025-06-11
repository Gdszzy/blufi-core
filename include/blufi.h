#ifndef _BLUFI_H
#define _BLUFI_H

#include "dh.h"
#include "msg.h"
#include <cstdint>
#include <deque>
#include <functional>
#include <span>
#include <string>
#include <variant>

/**
 * 发送请求和onReceiveData返回说明，1字节长度，前4位表示Type 后4位表示subType
 * 发送阶段错误：
 *  0xF0
 * 接受阶段错误：
 *  0x10 错误数据长度
 *  0x20 错误序号
 *  0x30 空数据
 *  0x40 缺少密钥
 *  0x50 非法Type
 *  0x60 操作类型不匹配
 *  0x70 远程错误 后四位为远程错误类型
 *  0x80 未实现功能
 */

namespace blufi {

using namespace std;

enum ErrorCode {
  SendError = 0xF0,
  WrongDataLen = 0x10,
  WrongSeq = 0x20,
  EmptyData = 0x30,
  KeyStateNotMatch = 0x40,
  InvalidType = 0x50,
  ActionNotMatch = 0x60,
  RemoteError = 0x70,
  NotImplement = 0x80
};

typedef struct {
  std::string ssid;
  std::int8_t rssi;
} Wifi;

// using NegotiateResult = std::function<void(int result)>;
// using ScanWifiResult = std::function<void(const std::vector<Wifi> &)>;
// using BytesResult = std::function<void(const std::span<uint8_t> &)>;

// using OnResult = std::variant<BytesResult, NegotiateResult, ScanWifiResult>;

using OnMessage =
    std::function<void(uint8_t type, uint8_t subType, void *data)>;

using SendData = std::vector<std::vector<uint8_t>>;

class Core {
public:
  Core(int mtu, OnMessage onMessage);
  ~Core();
  uint8_t onReceiveData(std::span<uint8_t>);

  // function are
  uint8_t negotiateKey(SendData &sendData);
  uint8_t custom(SendData &sendData, std::vector<uint8_t>);
  uint8_t scanWifi(SendData &sendData);
  uint8_t connectWifi(SendData &sendData, std::string ssid, std::string pass);

private:
  uint8_t *buffer;
  int mtu;
  std::span<uint8_t> bufferSpan;
  std::function<int(std::span<uint8_t>)> onSendData;
  uint8_t sendSeq = 0;
  uint8_t recvSeq = 0;
  // 16 bytes aes key
  uint8_t *key = NULL;

  dh::DH *tmpKey = NULL;

  std::vector<uint8_t> bodyBuffer;

  void fillSendData(msg::Msg &msg, SendData &sendData);

  OnMessage onMessage;
};
} // namespace blufi

#endif //_BLUFI_H