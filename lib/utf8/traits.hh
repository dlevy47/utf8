#pragma once

namespace utf8::string {

template <
    typename TCodepointType,
    typename TSizeType,
    typename TVector,
    typename TOptionalCodepoint>
struct Traits {
  using CodepointType     = TCodepointType;
  using SizeType          = TSizeType;
  using Vector            = TVector;
  using OptionalCodepoint = TOptionalCodepoint;

  constexpr Vector&& move_vector(Vector&& t) noexcept;
  SizeType           strlen(const char* s) noexcept;
};

} // namespace utf8::string
