#pragma once

#include <Arduino.h>
namespace CanBusData_asukiaaa {

class Frame {
 public:
  uint32_t id = 0;
  bool ext = false;
  bool rtr = false;  // false -> data frame, true -> remote frame
  uint8_t len = 8;
  union {
    uint64_t data64;     // Caution: subject to endianness
    uint32_t data32[2];  // Caution: subject to endianness
    uint16_t data16[4];  // Caution: subject to endianness
    float dataFloat[2];  // Caution: subject to endianness
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  };

  String toString() const;
};

};  // namespace CanBusData_asukiaaa
