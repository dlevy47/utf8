#pragma once

#include <optional>
#include <ostream>
#include <vector>

#include "string.hh"

namespace utf8 {

struct StringTraits {
  using CodepointType     = char32_t;
  using SizeType          = std::size_t;
  using Vector            = std::vector<uint8_t>;
  using OptionalCodepoint = std::optional<uint32_t>;
  using BaseException     = std::exception;

  constexpr static Vector&& move_vector(
      Vector& t) noexcept {
    return std::move(t);
  }

  static SizeType strlen(
      const char* s) {
    return std::strlen(s);
  }
};

using String = string::String<StringTraits>;

namespace string {
static utf8::String from_string_unchecked(
    const std::string& s) {
  return utf8::String::from_cstr_unchecked(s.c_str(), s.size() + 1);
}
} // namespace string

} // namespace utf8

inline std::ostream& operator<<(
    std::ostream&       os,
    const utf8::String& s) {
  return os << s.c_str();
}

inline std::ostream& operator<<(
    std::ostream&             os,
    const utf8::String::View& view) {
  auto cp {view.codepoints()};

  while (const auto c = cp.next()) {
    uint8_t encoded[5] {0};
    utf8::encode(encoded, *c);

    os << (const char*)encoded;
  }

  return os;
}

// std::hash specialization for String.
template <> struct std::hash<utf8::String> {
  std::size_t operator()(
      const utf8::String& s) const {
    return std::hash<std::string_view> {}(std::string_view {s.c_str()});
  }
};
