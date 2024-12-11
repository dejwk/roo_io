#include "roo_io/text/string_printf.h"

#include "gtest/gtest.h"

namespace roo_io {

TEST(StringPrintf, Examples) {
  EXPECT_EQ("2010-02-05 12:03",
            StringPrintf("%04d-%02d-%02d %02d:%02d", 2010, 02, 5, 12, 3));
  EXPECT_EQ("0x3F 0x0C 0x00",
            StringPrintf("0x%02X 0x%02X 0x%02X", 0x3F, 0x0C, 0x00));
  EXPECT_EQ(" 12.3°C", StringPrintf("%5.1f°C", 12.31));

  std::string_view s{"Hello this is longer then needed!"};
  auto sub = s.substr(0, 5);
  EXPECT_EQ("Hello", StringPrintf("%.*s", sub.length(), sub.data()));

  std::string s1 = "Hello";
  std::string s2 = "World";
  EXPECT_EQ("Hello, World!", StringPrintf("%s, %s!", s1.c_str(), s2.c_str()));
}

}  // namespace roo_io