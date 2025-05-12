#include <gtest/gtest.h>

#include <sstream>
#include <utf8/stlscanner.hh>
#include <utf8/stlstring.hh>

inline void string_eq(
    const std::string&        want,
    const utf8::String::View& got) {
  std::vector<uint32_t> got_codepoints {};

  auto codepoints = got.codepoints();
  while (auto codepoint = codepoints.next()) {
    got_codepoints.push_back(*codepoint);
  }

  EXPECT_EQ(want.size(), got_codepoints.size());
  for (size_t i = 0; i < got_codepoints.size(); ++i) {
    ASSERT_LE(got_codepoints[i], 0xFF);
    EXPECT_EQ(got_codepoints[i], want[i]);
  }
}

TEST(
    StringTests,
    FromCStr) {
  const char* const want = "hello world";

  const auto            string = utf8::String::from_cstr_unchecked(want);
  std::vector<uint32_t> got {};

  auto codepoints = string.codepoints();
  while (auto cp = codepoints.next()) {
    got.push_back(*cp);
  }

  EXPECT_EQ(strlen(want), got.size());
  for (size_t i = 0; i < got.size(); ++i) {
    ASSERT_LE(got[i], 0xFF);
    EXPECT_EQ(got[i], want[i]);
  }
}

TEST(
    StringTests,
    FromString) {
  const std::string want = "hello world";

  const auto            string = utf8::string::from_string_unchecked(want);
  std::vector<uint32_t> got {};

  string_eq(want, string);
}

TEST(
    StringTests,
    View) {
  const std::string want = "hello world";

  const auto string = utf8::string::from_string_unchecked(want);

  utf8::String::Codepoints to = string.codepoints();

  to.advance(5);
  string_eq("hello", string.view().to(to));

  auto from {to};
  from.advance(1);
  to.advance(6);

  ASSERT_EQ(string.view().from(from).codepoints().peek(), 'w');

  string_eq("world", string.view().from_to(from, to));
}

TEST(
    StringTests,
    Equal) {
  const auto string1 = utf8::String::from_cstr_unchecked("1234 word");
  const auto string2 = utf8::String::from_cstr_unchecked("1234 word");
  const auto string3 = utf8::String::from_cstr_unchecked("word");

  EXPECT_EQ(string1, string1);
  EXPECT_EQ(string1, string2);
  EXPECT_NE(string1, string3);
  EXPECT_EQ(string2, string1);
  EXPECT_EQ(string2, string2);
  EXPECT_NE(string2, string3);
  EXPECT_NE(string3, string1);
  EXPECT_NE(string3, string2);
  EXPECT_EQ(string3, string3);
}

TEST(
    StringTests,
    OStream) {
  const auto string1 = utf8::String::from_cstr_unchecked("1234 word");
  const auto string2 = utf8::String::from_cstr_unchecked("word");

  std::stringstream ss {};

  ss << string1;
  EXPECT_EQ(ss.str(), "1234 word");

  ss << string2;
  EXPECT_EQ(ss.str(), "1234 wordword");
}

TEST(
    ScannerTests,
    Scan) {
  const auto string = utf8::String::from_cstr_unchecked("1234 word");

  uint64_t     integer {0};
  utf8::String word {};
  ASSERT_EQ(
      utf8::scanner::Error::None,
      utf8::Scanner {string}.scan(
          &integer, utf8::scanner::Whitespace {}, &word));

  EXPECT_EQ(integer, 1234);
  string_eq("word", word);
}
