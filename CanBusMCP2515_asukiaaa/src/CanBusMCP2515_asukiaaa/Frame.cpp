#include "Frame.h"

namespace CanBusMCP2515_asukiaaa {

Frame::Frame() {}

Frame::Frame(const CanBusData_asukiaaa::Frame &frame) {
  id = frame.id;
  ext = frame.ext;
  len = frame.len;
  data64 = frame.data64;
};

}  // namespace CanBusMCP2515_asukiaaa
