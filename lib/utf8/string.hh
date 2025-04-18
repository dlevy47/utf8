#pragma once

#include "utf8.hh"

namespace utf8 {
namespace string {

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

template <typename Traits> class String {
public:
  using Codepoint         = typename Traits::CodepointType;
  using SizeType          = typename Traits::SizeType;
  using Vector            = typename Traits::Vector;
  using OptionalCodepoint = typename Traits::OptionalCodepoint;
  using move              = typename Traits::move;

  class Codepoints {
  public:
    const String& string;

    OptionalCodepoint next() {
      if (index >= string.data->size()) {
        return OptionalCodepoint {};
      }

      const char*       s     = string.data->c_str() + index;
      const char* const start = s;
      Codepoint         cp    = codepoint(&s);
      if (!s) {
        index = string.data->size();
        return OptionalCodepoint {};
      }

      index += start - s;

      return OptionalCodepoint {cp};
    }

  private:
    SizeType index {0};
  };

public:
  template <typename OtherTraits>
  String(
      const String<OtherTraits>& rhs)
      : data {rhs.data} {}

  template <typename OtherTraits>
  String(
      String<OtherTraits>&& rhs)
      : data {Traits::move_vector(rhs.data)} {}

  template <typename OtherTraits>
  String& operator=(
      const String<OtherTraits>& rhs) {
    data = rhs.data;
    return *this;
  }

  template <typename OtherTraits>
  String& operator=(
      String<OtherTraits>&& rhs) {
    if (this == &rhs) {
      return *this;
    }

    data = Traits::move_vector(rhs.data);
    return *this;
  }

  ~String() = default;

  static String from_cstr_unchecked(
      const char* cstr) {
    const SizeType len = Traits::strlen(cstr);

    return String {cstr, cstr + len + 1};
  }

  Codepoints codepoints() const { return Codepoints {*this}; }

private:
  Vector data {};

  String() = default;
  String(
      const char* start,
      const char* end)
      : data {
            start,
            end} {}
};

} // namespace string
} // namespace utf8
