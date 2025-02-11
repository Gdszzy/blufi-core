#ifndef _MSG_H
#define _MSG_H

#include <cstdint>
#include <span>
#include <vector>

namespace msg {
const uint8_t FRAME_CTRL_POSITION_ENCRYPTED = 0;
const uint8_t FRAME_CTRL_POSITION_CHECKSUM = 1;
const uint8_t FRAME_CTRL_POSITION_DATA_DIRECTION = 2;
const uint8_t FRAME_CTRL_POSITION_REQUIRE_ACK = 3;
const uint8_t FRAME_CTRL_POSITION_FRAG = 4;

enum Type { CONTROL_VALUE = 0x00, VALUE = 0x01 };
enum SubType {
  // value
  NEG = 0x00,
  SET_SEC_MODE = 0x01,
  SET_SSID = 0x02,
  SET_PWD = 0x03,
  CUSTOM_DATA = 0x13,
  WIFI_LIST_NEG = 0x11,
  WIFI_STATUS = 0x0F,
  ERROR = 0x12,
  // control value
  END = 0x03,
  WIFI_NEG = 0x09,
};
class Msg {
public:
  Msg(Type type, SubType SubType, std::span<uint8_t> data, bool needChecksum,
      uint8_t *key, std::span<uint8_t> buffer);
  size_t fillFrame(uint8_t seq);
  bool hasNext();

private:
  uint8_t msgType;
  std::span<uint8_t> data;
  uint8_t *key;
  std::span<uint8_t> buffer;
  bool needChecksum;
  // control
  bool end = false;
};
} // namespace msg

#endif //_MSG_H