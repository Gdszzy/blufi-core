#pragma once

#include <memory>
#include <span>
#include <vector>

namespace dh {

class DH {
public:
  DH();
  ~DH();

  std::vector<uint8_t> getPBytes();
  std::vector<uint8_t> getGBytes();
  std::vector<uint8_t> getPubBytes();
  std::vector<uint8_t> generateKey(std::span<uint8_t> remotePubKeyBytes);

private:
  struct Internal;
  std::unique_ptr<Internal> internal;
};
} // namespace dh
