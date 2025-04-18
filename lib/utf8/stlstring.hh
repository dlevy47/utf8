#pragma once

#include <optional>
#include <vector>

#include "string.hh"

namespace utf8 {

struct StringTraits {
  using Vector            = std::vector<uint8_t>;
  using OptionalCodepoint = std::optional<uint32_t>;

  constexpr Vector&& move_vector(
      Vector&& t) noexcept {
    return std::move(t);
  }
};

using String = string::String<StringTraits>;

} // namespace utf8
