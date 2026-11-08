#include <gtest/gtest.h>

#include <exception>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

#include <utf8/stlstring.hh>

TEST(
    StringTests,
    DefaultsToAnEmptyNullTerminatedString) {
  const utf8::String string {};

  EXPECT_FALSE(string.codepoints());
  EXPECT_STREQ(string.c_str(), "");
}

TEST(
    StringTests,
    ConstructsFromEmptyVector) {
  utf8::String::Vector bytes {};
  const utf8::String   string {std::move(bytes)};

  EXPECT_FALSE(string.codepoints());
  EXPECT_STREQ(string.c_str(), "");
}

TEST(
    StringTests,
    IteratesAsciiAndMultibyteCodepoints) {
  const auto string =
      utf8::String::from_cstr_unchecked("A\xC3\xA9\xF0\x9F\x98\x80");
  std::vector<uint32_t> codepoints {};

  auto iterator = string.codepoints();
  while (const auto codepoint = iterator.next()) {
    codepoints.push_back(*codepoint);
  }

  EXPECT_EQ(
      codepoints, (std::vector<uint32_t> {U'A', U'\u00E9', U'\U0001F600'}));
  EXPECT_FALSE(iterator.peek());
}

TEST(
    StringTests,
    PeekDoesNotAdvanceTheCursor) {
  const auto string   = utf8::String::from_cstr_unchecked("abc");
  auto       iterator = string.codepoints();

  EXPECT_EQ(iterator.peek(), U'a');
  EXPECT_EQ(iterator.peek(), U'a');
  EXPECT_EQ(iterator.next(), U'a');
  EXPECT_EQ(iterator.peek(), U'b');
}

TEST(
    StringTests,
    CopiesAndMovesStrings) {
  const auto source = utf8::String::from_cstr_unchecked("hello");
  const auto copy   = source;
  auto       moved  = utf8::String::from_cstr_unchecked("world");
  const auto target = std::move(moved);

  EXPECT_EQ(copy, source);
  EXPECT_STREQ(target.c_str(), "world");
}

TEST(
    StringTests,
    OrdersStringsLexicographically) {
  const auto before = utf8::String::from_cstr_unchecked("a");
  const auto equal  = utf8::String::from_cstr_unchecked("a");
  const auto after  = utf8::String::from_cstr_unchecked("b");

  EXPECT_LT(before, after);
  EXPECT_GT(after, before);
  EXPECT_FALSE(before < equal);
  EXPECT_FALSE(before > equal);
}

TEST(
    StringTests,
    StreamsStringsAndViews) {
  const auto string = utf8::String::from_cstr_unchecked("hello world");
  auto       end    = string.codepoints();
  end.advance(5);
  std::stringstream stream {};

  stream << string.view().to(end);
  stream << string;

  EXPECT_EQ(stream.str(), "hellohello world");
}

TEST(
    StringTests,
    HashesEqualStringsTheSame) {
  const auto first  = utf8::String::from_cstr_unchecked("same");
  const auto second = utf8::String::from_cstr_unchecked("same");

  EXPECT_EQ(
      std::hash<utf8::String> {}(first), std::hash<utf8::String> {}(second));
}

TEST(
    StringTests,
    RejectsReversedCodepointRange) {
  const auto string = utf8::String::from_cstr_unchecked("abc");

  auto start = string.codepoints();
  start.advance(2);
  auto end = string.codepoints();
  end.advance(1);

  EXPECT_THROW(utf8::String(start, end), utf8::String::OutOfRangeException);
}

TEST(
    StringTests,
    ExceptionsAreCatchableAsStandardExceptions) {
  EXPECT_TRUE((std::is_convertible_v<
               utf8::String::OutOfRangeException*, std::exception*>));
}

TEST(
    ViewTests,
    SlicesOnCodepointBoundaries) {
  const auto string =
      utf8::String::from_cstr_unchecked("hello \xF0\x9F\x8C\x8D world");
  const auto globe = utf8::String::from_cstr_unchecked("\xF0\x9F\x8C\x8D");
  auto       start = string.codepoints();
  start.advance(6);
  auto end = start;
  end.advance(1);

  EXPECT_EQ(string.view().from_to(start, end), globe.view());
}

TEST(
    ViewTests,
    StartsWithRequiresTheEntirePrefix) {
  const auto string = utf8::String::from_cstr_unchecked("alphabet");
  const auto prefix = utf8::String::from_cstr_unchecked("alpha");
  const auto longer = utf8::String::from_cstr_unchecked("alphabetical");
  const auto wrong  = utf8::String::from_cstr_unchecked("alps");

  EXPECT_TRUE(string.view().starts_with(prefix.view()));
  EXPECT_FALSE(string.view().starts_with(longer.view()));
  EXPECT_FALSE(string.view().starts_with(wrong.view()));
}

TEST(
    ViewTests,
    RejectsCodepointsFromAnotherString) {
  using CheckException = utf8::String::Checks::CheckException;

  const auto string = utf8::String::from_cstr_unchecked("abc");
  const auto other  = utf8::String::from_cstr_unchecked("def");
  auto       at     = other.codepoints();
  at.advance(1);

  EXPECT_THROW(string.view().to(at), CheckException);
}

TEST(
    ViewTests,
    HonorsDisabledChecks) {
  using CheckException = utf8::String::Checks::CheckException;

  struct StringTraits {
    using CodepointType     = char32_t;
    using SizeType          = std::size_t;
    using Vector            = std::vector<uint8_t>;
    using OptionalCodepoint = std::optional<uint32_t>;
    using BaseException     = std::exception;
    using Checks            = utf8::string::ChecksDisabled;

    constexpr static Vector&& move_vector(
        Vector& t) noexcept {
      return std::move(t);
    }

    static SizeType strlen(
        const char* s) {
      return std::strlen(s);
    }
  };

  using String = utf8::string::String<StringTraits>;

  const auto string = String::from_cstr_unchecked("abc");
  const auto other  = String::from_cstr_unchecked("def");
  auto       at     = other.codepoints();
  at.advance(1);

  EXPECT_NO_THROW(string.view().to(at));
}
