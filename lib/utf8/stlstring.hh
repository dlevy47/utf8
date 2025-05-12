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

inline std::ostream& operator<<(
    std::ostream& os,
    const String& s) {
  return os << s.c_str();
}

} // namespace utf8
