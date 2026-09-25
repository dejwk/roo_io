#include "roo_io/text/unicode.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io/memory/memory_output_iterator.h"

using testing::ElementsAre;

namespace roo_io {

// Verifies strict validation accepts Unicode scalar boundaries and replacement
// characters without changing the replacement-based decoder's behavior.
TEST(Utf8, StrictValidationAcceptsValidSequences) {
  EXPECT_TRUE(IsValidUtf8(""));
  EXPECT_TRUE(IsValidUtf8(roo::string_view("ASCII\0text", 10)));
  EXPECT_TRUE(IsValidUtf8("\xC2\x80\xDF\xBF"));
  EXPECT_TRUE(IsValidUtf8("\xE0\xA0\x80\xED\x9F\xBF\xEE\x80\x80"));
  EXPECT_TRUE(IsValidUtf8("\xF0\x90\x80\x80\xF4\x8F\xBF\xBF"));
  EXPECT_TRUE(IsValidUtf8("\xEF\xBF\xBD"));
}

// Verifies strict validation rejects malformed byte sequences rather than
// treating decoder replacement characters as valid input.
TEST(Utf8, StrictValidationRejectsMalformedSequences) {
  EXPECT_FALSE(IsValidUtf8("\x80"));
  EXPECT_FALSE(IsValidUtf8("\xC0\x80"));
  EXPECT_FALSE(IsValidUtf8("\xE0\x80\x80"));
  EXPECT_FALSE(IsValidUtf8("\xED\xA0\x80"));
  EXPECT_FALSE(IsValidUtf8("\xF0\x80\x80\x80"));
  EXPECT_FALSE(IsValidUtf8("\xF4\x90\x80\x80"));
  EXPECT_FALSE(IsValidUtf8("\xF8\x80\x80\x80\x80"));
  EXPECT_FALSE(IsValidUtf8("\xE2\x82"));
}

TEST(Utf8, DecodeValidUsingDecoder) {
  std::string in = "Pełżą 나는 유";
  Utf8Decoder decoder(in);
  std::vector<char32_t> output;
  char32_t ch;
  while (decoder.next(ch)) {
    output.push_back(ch);
  }
  EXPECT_THAT(output, ElementsAre(0x50, 0x65, 0x142, 0x17C, 0x105, 0x20, 0xB098,
                                  0xB294, 0x20, 0xC720));
}

TEST(Utf8, DecodeValid) {
  EXPECT_THAT(DecodeUtfStringToVector("\x20\x20"), ElementsAre(0x20, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("Pełżą 나는 유"),
              ElementsAre(0x50, 0x65, 0x142, 0x17C, 0x105, 0x20, 0xB098, 0xB294,
                          0x20, 0xC720));
}

TEST(Utf8, ProperlyRejectInvalid) {
  // We reject 'extended UTF-8' for now.
  EXPECT_THAT(DecodeUtfStringToVector("\xC0\x80"), ElementsAre(0xFFFD, 0xFFFD));
  // Non-minimal sequences.
  EXPECT_THAT(DecodeUtfStringToVector("\xC0\x20"), ElementsAre(0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xE0\x80\x80\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xE0\x80\x80\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xF0\x80\x80\x80\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  // UTF-16 surrogates.
  EXPECT_THAT(DecodeUtfStringToVector("\xED\xA0\x80\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xED\xAF\xBF\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xED\xB0\x80\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xED\xBF\xBF\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  // Some boundary conditions.
  EXPECT_THAT(DecodeUtfStringToVector("\xEF\xBF\xBD\x20"),
              ElementsAre(0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xF4\x90\x80\x80\x20"),
              ElementsAre(0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x20));
  // Lonely start characters.
  EXPECT_THAT(DecodeUtfStringToVector("\x80\x20\xC8\x20"),
              ElementsAre(0xFFFD, 0x20, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xF3\x20\xF8\x20\xFC\x20"),
              ElementsAre(0xFFFD, 0x20, 0xFFFD, 0x20, 0xFFFD, 0x20));
  // Unterminated sequences.
  EXPECT_THAT(DecodeUtfStringToVector("\xe2\x28\xa1\x20"),
              ElementsAre(0xFFFD, 0x28, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xe2\x82\x28\x20"),
              ElementsAre(0xFFFD, 0x28, 0x20));

  EXPECT_THAT(DecodeUtfStringToVector("\xf0\x28\x8c\xbc\x20"),
              ElementsAre(0xFFFD, 0x28, 0xFFFD, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xf0\x90\x28\xbc\x20"),
              ElementsAre(0xFFFD, 0x28, 0xFFFD, 0x20));
  EXPECT_THAT(DecodeUtfStringToVector("\xf0\x90\x8c\x28\x20"),
              ElementsAre(0xFFFD, 0x28, 0x20));
}

TEST(Utf8, WriteeUtf8Char) {
  uint8_t result[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr{(byte*)result, (byte*)result + 8};
  WriteUtf8Char(itr, 'f');
  WriteUtf8Char(itr, 0x0123);
  WriteUtf8Char(itr, 0x54d5);
  ASSERT_EQ(kOk, itr.status());
  EXPECT_THAT(result, ElementsAre('f', 0xC4, 0xA3, 0xE5, 0x93, 0x95, 9, 9));
}

}  // namespace roo_io
