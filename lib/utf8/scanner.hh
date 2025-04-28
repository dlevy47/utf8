#pragma once

#include "string.hh"
#include "utf8/utf8.hh"

namespace utf8 {
namespace scanner {

enum struct Error {
  None,
  EndOfFile,
  NumericOverflow,
};

template <typename Traits, typename T> struct Scan {
  static Error scan(
      T                                   t,
      string::String<Traits>::Codepoints& codepoints) {
    static_assert(false, "unimplemented");
  }
};

template <typename Integral, int Base> struct UnsignedInteger {
  Integral* into;
};

struct Whitespace {};

template <typename Traits> struct Scanner {
public:
  string::String<Traits>::Codepoints codepoints;

  Scanner(
      const string::String<Traits>& s)
      : codepoints {s.codepoints()} {}
  Scanner(
      const string::String<Traits>::View& v)
      : codepoints {v.codepoints()} {}
  Scanner(
      const string::String<Traits>::Codepoints& c)
      : codepoints {c} {}

  Error scan() { return Error::None; }

  template <
      typename T,
      typename... Args>
  Error scan(
      T t,
      Args... args) {
    const Error err = Scan<Traits, T>::scan(t, codepoints);
    if (err != Error::None) {
      return err;
    }

    return scan(args...);
  }
};

template <typename Traits, int Base> struct NumericBase {
  static bool valid(
      Traits::CodepointType codepoint) {
    static_assert(false, "unimplemented");
  }

  static unsigned int convert(
      Traits::CodepointType codepoint) {
    static_assert(false, "unimplemented");
  }
};

template <typename Traits> struct NumericBase<Traits, 10> {
  static bool valid(
      Traits::CodepointType codepoint) {
    return codepoint >= '0' && codepoint <= '9';
  }

  static unsigned int convert(
      Traits::CodepointType codepoint) {
    return codepoint - '0';
  }
};

template <typename Traits> struct Scan<Traits, Whitespace> {
  static Error scan(
      Whitespace                          into,
      string::String<Traits>::Codepoints& codepoints) {
    while (auto cp = codepoints.peek()) {
      if (!is_whitespace(*cp)) {
        break;
      }

      codepoints.next();
    }

    return Error::None;
  }
};

template <typename Traits> struct Scan<Traits, string::String<Traits>*> {
  static Error scan(
      string::String<Traits>*             into,
      string::String<Traits>::Codepoints& codepoints) {
    typename Traits::Vector ret {};
    while (auto cp = codepoints.next()) {
      ret.push_back(*cp);
    }

    *into = string::String<Traits> {Traits::move_vector(ret)};

    return Error::None;
  }
};

template <typename Traits, typename Integral, int Base>
struct Scan<Traits, UnsignedInteger<Integral, Base>> {
  using NumericBase = NumericBase<Traits, Base>;

  static Error scan(
      UnsignedInteger<
          Integral,
          Base>                           into,
      string::String<Traits>::Codepoints& codepoints) {
    Integral MAX {0};
    MAX = ~MAX;

    Integral value {0};
    bool     started {false};
    while (auto cp = codepoints.peek()) {
      if (!NumericBase::valid(*cp)) {
        break;
      }

      cp      = codepoints.next();
      started = true;

      if (value > 0 && MAX / value < Base) {
        return Error::NumericOverflow;
      }

      value = (value * Base) + NumericBase::convert(*cp);
    }

    if (!started) {
      return Error::EndOfFile;
    }

    *into.into = value;
    return Error::None;
  }
};

} // namespace scanner
} // namespace utf8
