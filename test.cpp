#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace msg {
extern uint16_t caluCRC(std::vector<uint8_t>);
}

int main() {
  uint8_t bytes[] = {0x04, 0x08, 0xbd, 0xa0, 0x00,
                     0x02, 0x01, 0x13, 0x0a, 0xdb};
  std::vector<uint8_t> buf;
  buf.insert(buf.end(), bytes, bytes + sizeof(bytes));
  uint16_t crc = msg::caluCRC(buf);
  std::cout << std::to_string(crc) << std::endl;
  printf("%04x\n", crc);
  return 0;
}