#pragma once

#if (defined ARDUINO)

#include "Arduino.h"
#include "roo_io/stream/arduino/stream_output_stream.h"
#include "roo_logging.h"

namespace roo_io {

/// Adapter exposing Arduino serial as `OutputStream`.
///
/// Delegates to `ArduinoStreamOutputStream`.
class ArduinoSerialOutputStream : public ArduinoStreamOutputStream {
 public:
  template <typename SerialType>
  ArduinoSerialOutputStream(SerialType& output)
      : ArduinoStreamOutputStream(output) {}
};

}  // namespace roo_io

#endif  // (defined ARDUINO)
