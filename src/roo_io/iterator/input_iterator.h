#pragma once

#include <cstdint>
#include "roo_io/status.h"

// The byte iterator template contract abstracts away various sources of
// data, such as: DRAM, PROGMEM, SPIFFS, or SDFS. A compliant class looks like
// follows:

// class MyIterator {
//  public:
//   // Reads a single byte.
//   uint8_t read();
//
//   // (Optional) Skips over the specified count of bytes. If not provided,
//   // will be effectively substituted in the client code by
//   // `while (offset-- > 0) read();`
//   void skip(unsigned int count);
//
//   // Returns kOk  if the last read or skip operation succeeded. Returns a
//   // different status code, as appropriate, when it failed, e.g. because of
//   // encountering EOS or a read error.
//   Status status() const;
// };
