#include <gtest/gtest.h>

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
