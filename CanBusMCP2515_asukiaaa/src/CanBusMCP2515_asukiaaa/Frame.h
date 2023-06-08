#pragma once

#include <CanBusData_asukiaaa.h>

namespace CanBusMCP2515_asukiaaa {

class Frame : public CanBusData_asukiaaa::Frame {
 public:
  uint8_t idx = 0;
  Frame();
  Frame(const CanBusData_asukiaaa::Frame &frame);
};

}  // namespace CanBusMCP2515_asukiaaa
