#pragma once

#include <stdint.h>

#include "roo_io/status.h"

// The output iterator template contract abstracts away various sinks of
// data, such as: DRAM, SPIFFS, or SDFS. A compliant class looks like
// follows:

// class MyOutputIterator {
//  public:
//   // Writes a single byte.
//   void write(uint8_t v);
//
//   // Returns kOk if the last read or seek operation succeeded. Returns a
//   // different status, as appropriate, if it failed, e.g. because of
//   // encountering EOS or a read error.
//   Status status() const;
// };
