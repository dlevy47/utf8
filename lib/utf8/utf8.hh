#pragma once

#ifdef UTF8_INTEGRAL_TYPES
#define utf8_u8 UTF8_UINT8_T
#define utf8_u32 UTF8_UINT32_T
#define utf8_usize UTF8_ISIZE_T
#else
#include <cstddef>
#include <cstdint>
#define utf8_u8 uint8_t
#define utf8_u32 uint32_t
#define utf8_usize size_t
#endif

namespace utf8 {

enum : utf8_u8 {
  ASCII     = 0b01111111,
  TWOBYTE   = 0b11000000,
  THREEBYTE = 0b11100000,
  FOURBYTE  = 0b11110000,
  MASK      = 0b00111111,

  CONTINUE       = 0b10000000,
  CONTINUE_START = 0b10000000,
  CONTINUE_END   = 0b11000000,
};

template <const utf8_usize N>
utf8_usize encode(
    utf8_u8 (&bytes)[N],
    const utf8_u32 codepoint) {
  static_assert(N >= 4, "buffer must be big enough for encoded codepoint");

  if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
    return 0;
  }

  if (codepoint >= 0x0 && codepoint < 0x80) {
    bytes[0] = static_cast<utf8_u8>(codepoint);

    return 1;
  } else if (codepoint >= 0x80 && codepoint < 0x800) {
    bytes[0] = TWOBYTE | (static_cast<utf8_u8>(codepoint >> 6) & 0b11111);
    bytes[1] = CONTINUE | (static_cast<utf8_u8>(codepoint) & MASK);

    return 2;
  } else if (codepoint >= 0x800 && codepoint < 0x10000) {
    bytes[0] = THREEBYTE | (static_cast<utf8_u8>(codepoint >> 12) & 0b1111);
    bytes[1] = CONTINUE | (static_cast<utf8_u8>(codepoint >> 6) & MASK);
    bytes[2] = CONTINUE | (static_cast<utf8_u8>(codepoint) & MASK);

    return 3;
  } else if (codepoint >= 0x10000 && codepoint < 0x110000) {
    bytes[0] = FOURBYTE | (static_cast<utf8_u8>(codepoint >> 18) & 0b111);
    bytes[1] = CONTINUE | (static_cast<utf8_u8>(codepoint >> 12) & MASK);
    bytes[2] = CONTINUE | (static_cast<utf8_u8>(codepoint >> 6) & MASK);
    bytes[3] = CONTINUE | (static_cast<utf8_u8>(codepoint) & MASK);

    return 4;
  }

  return 0;
}

inline bool continues(
    const utf8_u8 x) {
  return (x >= CONTINUE_START) && (x < CONTINUE_END);
}

inline utf8_u32 codepoint(
    utf8_usize&          advance,
    const utf8_u8* const s) {
  do {
    utf8_u32 codepoint = static_cast<utf8_u32>(*s);
    if (codepoint == 0) {
      return 0;
    }

    if ((codepoint & FOURBYTE) == FOURBYTE) {
      if (!continues(s[1]) || !continues(s[2]) || !continues(s[3])) {
        break;
      }

      codepoint = static_cast<utf8_u32>(codepoint & ~FOURBYTE) << 18 |
                  static_cast<utf8_u32>(s[1] & MASK) << 12 |
                  static_cast<utf8_u32>(s[2] & MASK) << 6 |
                  static_cast<utf8_u32>(s[3] & MASK);

      if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
        break;
      }

      advance = 4;
      return codepoint;
    } else if ((codepoint & THREEBYTE) == THREEBYTE) {
      if (!continues(s[1]) || !continues(s[2])) {
        break;
      }

      codepoint = static_cast<utf8_u32>(codepoint & ~THREEBYTE) << 12 |
                  static_cast<utf8_u32>(s[1] & MASK) << 6 |
                  static_cast<utf8_u32>(s[2] & MASK);

      if (codepoint < 0x0800 || codepoint > 0xFFFF) {
        break;
      }

      if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
        break;
      }

      advance = 3;
      return codepoint;
    } else if ((codepoint & TWOBYTE) == TWOBYTE) {
      if (!continues(s[1])) {
        break;
      }

      codepoint = static_cast<utf8_u32>(codepoint & ~TWOBYTE) << 6 |
                  static_cast<utf8_u32>(s[1] & MASK);

      if (codepoint < 0x0080 || codepoint > 0x07FF) {
        break;
      }

      advance = 2;
      return codepoint;
    } else if ((codepoint | ASCII) == ASCII) {
      if (codepoint > 0x7F) {
        break;
      }

      advance = 1;
      return codepoint;
    }
  } while (false);

  advance = 0;
  return 0;
}

inline bool is_whitespace(
    const utf8_u32 x) {
  // List of characters from https://en.wikipedia.org/wiki/Whitespace_character
  return x == 0x0009 || x == 0x000A || x == 0x000B || x == 0x000C ||
         x == 0x000D || x == 0x0020 || x == 0x0085 || x == 0x00A0 ||
         x == 0x1680 || x == 0x180E || x == 0x2000 || x == 0x2001 ||
         x == 0x2002 || x == 0x2003 || x == 0x2004 || x == 0x2005 ||
         x == 0x2006 || x == 0x2007 || x == 0x2008 || x == 0x2009 ||
         x == 0x200A || x == 0x200B || x == 0x200C || x == 0x200D ||
         x == 0x2028 || x == 0x2029 || x == 0x202F || x == 0x205F ||
         x == 0x2060 || x == 0x3000;
}

} // namespace utf8

#undef utf8_u8
#undef utf8_u32
#undef utf8_usize
