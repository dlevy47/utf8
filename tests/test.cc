#include <gtest/gtest.h>

#include <utf8/stlstring.hh>

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

  auto codepoints = string.codepoints();
  while (auto cp = codepoints.next()) {
    got.push_back(*cp);
  }

  EXPECT_EQ(want.size(), got.size());
  for (size_t i = 0; i < got.size(); ++i) {
    ASSERT_LE(got[i], 0xFF);
    EXPECT_EQ(got[i], want[i]);
  }
}
