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

// String is an immutable utf8 encoded string. String's contents are generally
// guaranteed to always be valid null terminated utf8 with no internal nulls.
// Note that these guarantees are only fully held when creation of a String is
// performed via a method not ending in `_unchecked`.
//
// Because it is not valid for general utf8 strings to be accessed by byte
// indices, String does not provide any methods for random codepoint access.
template <typename Traits> class String {
public:
  using Codepoint         = typename Traits::CodepointType;
  using SizeType          = typename Traits::SizeType;
  using Vector            = typename Traits::Vector;
  using OptionalCodepoint = typename Traits::OptionalCodepoint;
  using BaseException     = typename Traits::BaseException;

  class Exception : BaseException {
  public:
    Exception(
        const char* what)
        : what_ {what} {}

    virtual const char* what() const noexcept { return what_; }

  private:
    const char* what_ {nullptr};
  };

  class OutOfRangeException : public Exception {
    using Exception::Exception;
  };

  class Codepoints {
  public:
    const String& string;

    Codepoints(const Codepoints&) = default;

    Codepoints(
        const String& string)
        : string {string} {}

    OptionalCodepoint peek() const {
      if (index_ >= end_) {
        return OptionalCodepoint {};
      }

      auto     s       = string.data.data() + index_;
      SizeType advance = 0;

      Codepoint cp = codepoint(advance, s);
      if (advance == 0) {
        return OptionalCodepoint {};
      }

      return OptionalCodepoint {cp};
    }

    OptionalCodepoint next() {
      if (index_ >= end_) {
        return OptionalCodepoint {};
      }

      auto     s       = string.data.data() + index_;
      SizeType advance = 0;

      Codepoint cp = codepoint(advance, s);
      if (advance == 0) {
        index_ = string.data.size();
        return OptionalCodepoint {};
      }

      index_ += advance;

      return OptionalCodepoint {cp};
    }

    Codepoints& advance(
        const SizeType count) {
      for (SizeType i = 0; i < count; ++i) {
        const auto cp = next();
        if (!cp) {
          return *this;
        }
      }

      return *this;
    }

    explicit operator bool() const { return index_ < end_; }

  private:
    Codepoints(
        const String&  string,
        const SizeType index,
        const SizeType end)
        : string {string},
          index_ {index},
          end_ {end} {}

    Codepoints(
        const String&  string,
        const SizeType index)
        : string {string},
          index_ {index},
          end_ {string.data.size() - 1} {}

    SizeType index_ {0};
    SizeType end_;

    friend class String;
    friend class View;
  };

  class View {
  public:
    const String& string;

    View(
        const String& string)
        : string {string},
          start_ {0},
          end_ {string.data.size() - 1} {}

    Codepoints codepoints() const { return Codepoints {string, start_, end_}; }

    View from(
        const Codepoints& from) const {
      return View {string, from.index_, end_};
    }

    View to(
        const Codepoints& to) const {
      return View {string, start_, to.index_};
    }

    View from_to(
        const Codepoints& from,
        const Codepoints& to) const {
      if (to.index_ < from.index_) {
        throw OutOfRangeException {"invalid index"};
      }

      return View {string, from.index_, to.index_};
    }

  private:
    SizeType start_;
    SizeType end_;

    View(
        const String&  string,
        const SizeType start)
        : string {string},
          start_ {start},
          end_ {string.data.size() - 1} {}

    View(
        const String&  string,
        const SizeType start,
        const SizeType end)
        : string {string},
          start_ {start},
          end_ {end} {
      if (end < start) {
        throw OutOfRangeException {"negative range"};
      }

      if (start >= string.data.size()) {
        throw OutOfRangeException {"invalid start"};
      }

      if (end >= string.data.size()) {
        throw OutOfRangeException {"invalid end"};
      }
    }

    friend class Codepoints;
    friend class String;
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

    return from_cstr_unchecked(cstr, len + 1);
  }

  static String from_cstr_unchecked(
      const char*    cstr,
      const SizeType len) {
    return String {cstr, cstr + len};
  }

  Codepoints codepoints() const { return Codepoints {*this}; }
  View       view() const { return View {*this}; }

private:
  Vector data {};

  void normalize_() {
    if (data.back() != '\0') {
      data.push_back('\0');
    }
  }

  String() = default;
  String(
      const char* start,
      const char* end)
      : data {
            start,
            end} {
    normalize_();
  }
};

} // namespace string
} // namespace utf8
