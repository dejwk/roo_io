#include "roo_io/fs/arduino/arduino_file_output_iterator.h"

#include "fakefs.h"
#include "fakefs_arduino.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "output_iterator_p.h"
#include "roo_io/fs/arduino/arduino_sdfs.h"

namespace roo_io {

class ArduinoFileOutputIteratorFixture {
 public:
  ArduinoFileOutputIterator createIterator(size_t max_size) {
    fake_ = std::unique_ptr<fakefs::FakeFs>(new fakefs::FakeFs(max_size));
    sdfs_ = std::unique_ptr<fakefs::FakeArduinoSdFsImpl>(
        new fakefs::FakeArduinoSdFsImpl(*fake_));
    return ArduinoFileOutputIterator(sdfs_->open("/foo", "w"));
  }

  std::string getResult() const { return fakefs::ReadTextFile(*fake_, "/foo"); }

  static constexpr bool strict = false;

 private:
  std::unique_ptr<fakefs::FakeFs> fake_;
  std::unique_ptr<fakefs::FakeArduinoSdFsImpl> sdfs_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(ArduinoFileOutputIteratorTest,
                               OutputIteratorTest,
                               ArduinoFileOutputIteratorFixture);

}  // namespace roo_io
