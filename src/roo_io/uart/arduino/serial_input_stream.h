#pragma once

#if (defined ARDUINO)

#include "Arduino.h"
#include "roo_io/core/input_stream.h"
#include "roo_io/stream/arduino/stream_input_stream.h"

namespace roo_io {

/// Adapter exposing Arduino serial as `InputStream`.
///
/// Delegates to `ArduinoStreamInputStream`.
class ArduinoSerialInputStream : public ArduinoStreamInputStream {
 public:
  template <typename SerialType>
  ArduinoSerialInputStream(SerialType& input)
      : ArduinoStreamInputStream(input) {}
};

}  // namespace roo_io

#endif  // (defined ARDUINO)
