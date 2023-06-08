#include "CanBusData_asukiaaa.h"
#include <string_asukiaaa.h>

namespace CanBusData_asukiaaa {

String Frame::toString() const {
  String str = "bytes:";
  for (int i = 0; i < len; ++i) {
    str += " " + String(data[i]);
  }
  return "id: " + String(id) + ", rtr: " + string_asukiaaa::trueFalse(rtr) +
         ", ext: " + string_asukiaaa::trueFalse(ext) + ", " + str;
};
};  // namespace CanBusData_asukiaaa
