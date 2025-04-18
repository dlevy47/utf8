#pragma once

#include <optional>
#include <vector>

#include "string.hh"

namespace utf8 {

struct StringTraits {
  using CodepointType     = std::uint32_t;
  using SizeType          = std::size_t;
  using Vector            = std::vector<uint8_t>;
  using OptionalCodepoint = std::optional<uint32_t>;

  constexpr static Vector&& move_vector(
      Vector&& t) noexcept {
    return std::move(t);
  }

  static SizeType strlen(
      const char* s) {
    return std::strlen(s);
  }
};

using String = string::String<StringTraits>;

} // namespace utf8
