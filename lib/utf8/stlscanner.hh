#pragma once

#include "scanner.hh"
#include "stlstring.hh"

namespace utf8 {

using ScanError = scanner::Error;
using Scanner   = scanner::Scanner<StringTraits>;

namespace scanner {

template <typename Traits> struct Scan<Traits, uint64_t*> {
  using Next = UnsignedInteger<uint64_t, 10>;

  static Error scan(
      uint64_t*                           t,
      string::String<Traits>::Codepoints& codepoints) {
    return Scan<Traits, Next>::scan(
        Next {
            .into = t,
        },
        codepoints);
  }
};

}; // namespace scanner

} // namespace utf8
