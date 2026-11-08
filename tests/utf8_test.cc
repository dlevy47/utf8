#include <gtest/gtest.h>

#include <initializer_list>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <utf8/utf8.hh>

namespace {

std::string format_bytes(
    const uint8_t* const bytes,
    const size_t         size) {
  std::ostringstream stream {};
  stream << '[';

  bool first {true};
  for (size_t i = 0; i < size; ++i) {
    if (!first) {
      stream << ", ";
    }

    stream << "0x" << std::hex << std::uppercase << std::setfill('0')
           << std::setw(2) << static_cast<unsigned int>(bytes[i]);
    first = false;
  }

  return stream.str() + ']';
}

std::string format_bytes(
    const std::initializer_list<uint8_t> bytes) {
  return format_bytes(bytes.begin(), bytes.size());
}

std::string format_codepoint(
    const uint32_t codepoint) {
  std::ostringstream stream {};
  stream << "U+" << std::hex << std::uppercase << std::setfill('0')
         << std::setw(4) << codepoint;

  return stream.str();
}

testing::AssertionResult check_encoding(
    const uint32_t                       codepoint,
    const std::initializer_list<uint8_t> expected) {
  uint8_t      bytes[4] {};
  const size_t encoded_size = utf8::encode(bytes, codepoint);
  const size_t displayed_size =
      encoded_size > sizeof(bytes) ? sizeof(bytes) : encoded_size;

  if (encoded_size != expected.size()) {
    return testing::AssertionFailure()
           << "codepoint: " << format_codepoint(codepoint) << '\n'
           << "  expected size: " << expected.size() << '\n'
           << "    actual size: " << encoded_size << '\n'
           << "  expected bytes: " << format_bytes(expected) << '\n'
           << "    actual bytes: " << format_bytes(bytes, displayed_size);
  }

  size_t i {0};
  for (const uint8_t byte : expected) {
    if (bytes[i] != byte) {
      return testing::AssertionFailure()
             << "codepoint: " << format_codepoint(codepoint) << '\n'
             << "  expected bytes: " << format_bytes(expected) << '\n'
             << "    actual bytes: " << format_bytes(bytes, displayed_size);
    }

    ++i;
  }

  return testing::AssertionSuccess();
}

testing::AssertionResult check_decoding(
    const std::initializer_list<uint8_t> bytes,
    const uint32_t                       expected) {
  std::vector<uint8_t> nul_terminated {bytes};
  nul_terminated.push_back(0);
  nul_terminated.push_back(0);
  nul_terminated.push_back(0);
  nul_terminated.push_back(0);

  size_t         advance {0};
  const uint32_t codepoint = utf8::codepoint(advance, nul_terminated.data());

  if (codepoint == expected && advance == bytes.size()) {
    return testing::AssertionSuccess();
  }

  return testing::AssertionFailure()
         << "bytes: " << format_bytes(bytes) << '\n'
         << "  expected codepoint: " << format_codepoint(expected) << '\n'
         << "    actual codepoint: " << format_codepoint(codepoint) << '\n'
         << "  expected advance: " << bytes.size() << '\n'
         << "    actual advance: " << advance;
}

testing::AssertionResult check_invalid_decoding(
    const std::initializer_list<uint8_t> bytes) {
  std::vector<uint8_t> nul_terminated {bytes};
  nul_terminated.push_back(0);
  nul_terminated.push_back(0);
  nul_terminated.push_back(0);
  nul_terminated.push_back(0);

  size_t         advance {99};
  const uint32_t codepoint = utf8::codepoint(advance, nul_terminated.data());

  if (codepoint == 0 && advance == 0) {
    return testing::AssertionSuccess();
  }

  return testing::AssertionFailure()
         << "bytes: " << format_bytes(bytes) << '\n'
         << "  expected codepoint: invalid (0)\n"
         << "    actual codepoint: " << format_codepoint(codepoint) << '\n'
         << "  expected advance: 0\n"
         << "    actual advance: " << advance;
}

} // namespace

TEST(
    Utf8EncodingTests,
    EncodesUnicodeBoundaryCodepoints) {
  EXPECT_TRUE(check_encoding(0x0000, {0x00}));
  EXPECT_TRUE(check_encoding(0x007F, {0x7F}));
  EXPECT_TRUE(check_encoding(0x0080, {0xC2, 0x80}));
  EXPECT_TRUE(check_encoding(0x07FF, {0xDF, 0xBF}));
  EXPECT_TRUE(check_encoding(0x0800, {0xE0, 0xA0, 0x80}));
  EXPECT_TRUE(check_encoding(0xFFFF, {0xEF, 0xBF, 0xBF}));
  EXPECT_TRUE(check_encoding(0x10000, {0xF0, 0x90, 0x80, 0x80}));
  EXPECT_TRUE(check_encoding(0x10FFFF, {0xF4, 0x8F, 0xBF, 0xBF}));
}

TEST(
    Utf8EncodingTests,
    RejectsNonUnicodeCodepoints) {
  uint8_t bytes[4] {};

  EXPECT_EQ(utf8::encode(bytes, 0xD800), 0);
  EXPECT_EQ(utf8::encode(bytes, 0xDFFF), 0);
  EXPECT_EQ(utf8::encode(bytes, 0x110000), 0);
}

TEST(
    Utf8DecodingTests,
    DecodesUnicodeBoundaryCodepoints) {
  EXPECT_TRUE(check_decoding({0x7F}, 0x007F));
  EXPECT_TRUE(check_decoding({0xC2, 0x80}, 0x0080));
  EXPECT_TRUE(check_decoding({0xDF, 0xBF}, 0x07FF));
  EXPECT_TRUE(check_decoding({0xE0, 0xA0, 0x80}, 0x0800));
  EXPECT_TRUE(check_decoding({0xEF, 0xBF, 0xBF}, 0xFFFF));
  EXPECT_TRUE(check_decoding({0xF0, 0x90, 0x80, 0x80}, 0x10000));
  EXPECT_TRUE(check_decoding({0xF4, 0x8F, 0xBF, 0xBF}, 0x10FFFF));
}

TEST(
    Utf8DecodingTests,
    RejectsTruncatedSequences) {
  EXPECT_TRUE(check_invalid_decoding({0xC2}));
  EXPECT_TRUE(check_invalid_decoding({0xE0, 0xA0}));
  EXPECT_TRUE(check_invalid_decoding({0xF0, 0x90, 0x80}));
}

TEST(
    Utf8DecodingTests,
    RejectsMalformedAndNonUnicodeSequences) {
  EXPECT_TRUE(check_invalid_decoding({0x80}));
  EXPECT_TRUE(check_invalid_decoding({0xC0, 0x80}));
  EXPECT_TRUE(check_invalid_decoding({0xC2, 0x41}));
  EXPECT_TRUE(check_invalid_decoding({0xE0, 0x80, 0x80}));
  EXPECT_TRUE(check_invalid_decoding({0xED, 0xA0, 0x80}));
  EXPECT_TRUE(check_invalid_decoding({0xF0, 0x80, 0x80, 0x80}));
  EXPECT_TRUE(check_invalid_decoding({0xF4, 0x90, 0x80, 0x80}));
  EXPECT_TRUE(check_invalid_decoding({0xF8, 0x80, 0x80, 0x80}));
}

TEST(
    WhitespaceTests,
    RecognizesEverySupportedWhitespaceCodepoint) {
  const std::vector<uint32_t> whitespace {
      0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x0085, 0x00A0,
      0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
      0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x200B, 0x200C, 0x200D,
      0x2028, 0x2029, 0x202F, 0x205F, 0x2060, 0x3000,
  };

  for (const uint32_t codepoint : whitespace) {
    EXPECT_TRUE(utf8::is_whitespace(codepoint));
  }
}

TEST(
    WhitespaceTests,
    RejectsNonWhitespaceCodepoints) {
  EXPECT_FALSE(utf8::is_whitespace(0x0000));
  EXPECT_FALSE(utf8::is_whitespace(U'a'));
  EXPECT_FALSE(utf8::is_whitespace(0x200E));
  EXPECT_FALSE(utf8::is_whitespace(0x10FFFF));
}
