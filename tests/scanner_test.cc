#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>

#include <utf8/stlscanner.hh>

TEST(
    ScannerTests,
    ScansUnsignedIntegersAtTheSupportedBounds) {
  const auto zero = utf8::String::from_cstr_unchecked("0");
  const auto maximum =
      utf8::String::from_cstr_unchecked("18446744073709551615");
  uint64_t value {99};

  EXPECT_EQ(utf8::Scanner {zero}.scan(&value), utf8::ScanError::None);
  EXPECT_EQ(value, 0);
  EXPECT_EQ(utf8::Scanner {maximum}.scan(&value), utf8::ScanError::None);
  EXPECT_EQ(value, std::numeric_limits<uint64_t>::max());
}

TEST(
    ScannerTests,
    ReportsOverflowFromTheFinalDigit) {
  const auto string = utf8::String::from_cstr_unchecked("18446744073709551616");
  uint64_t   value {0};

  EXPECT_EQ(
      utf8::Scanner {string}.scan(&value), utf8::ScanError::NumericOverflow);
}

TEST(
    ScannerTests,
    DoesNotModifyOutputWhenNoIntegerIsPresent) {
  const auto string = utf8::String::from_cstr_unchecked("word");
  uint64_t   value {42};

  EXPECT_EQ(utf8::Scanner {string}.scan(&value), utf8::ScanError::EndOfFile);
  EXPECT_EQ(value, 42);
}

TEST(
    ScannerTests,
    LeavesTheFirstNonNumericCodepointForTheNextScan) {
  const auto    string = utf8::String::from_cstr_unchecked("123 word");
  uint64_t      number {0};
  utf8::String  word {};
  utf8::Scanner scanner {string};

  ASSERT_EQ(scanner.scan(&number), utf8::ScanError::None);
  ASSERT_EQ(
      scanner.scan(utf8::scanner::Whitespace {}, &word), utf8::ScanError::None);

  EXPECT_EQ(number, 123);
  EXPECT_STREQ(word.c_str(), "word");
}

TEST(
    ScannerTests,
    SkipsUnicodeWhitespace) {
  const auto string = utf8::String::from_cstr_unchecked(
      "\xC2\xA0"
      "42");
  uint64_t value {0};

  EXPECT_EQ(
      utf8::Scanner {string}.scan(utf8::scanner::Whitespace {}, &value),
      utf8::ScanError::None);
  EXPECT_EQ(value, 42);
}

TEST(
    ScannerTests,
    PreservesNonAsciiTextWhenScanningAString) {
  const std::string bytes {"\xC3\xA9"};
  const auto        string = utf8::string::from_string_unchecked(bytes);
  utf8::String      result {};

  ASSERT_EQ(utf8::Scanner {string}.scan(&result), utf8::ScanError::None);
  EXPECT_EQ(std::string {result.c_str()}, bytes);
}

TEST(
    ScannerTests,
    ScansAnEmptyString) {
  const utf8::String string {};
  utf8::String       result {};

  ASSERT_EQ(utf8::Scanner {string}.scan(&result), utf8::ScanError::None);
  EXPECT_STREQ(result.c_str(), "");
}
