#pragma once

#include "utf8.hh"

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

  // Exception is the base exception that all String exceptions inherit from.
  class Exception : BaseException {
  public:
    Exception(
        const char* what)
        : what_ {what} {}

    virtual const char* what() const noexcept override { return what_; }

  private:
    const char* what_ {nullptr};
  };

  // OutOfRangeException is thrown when a [start, end) interval is not valid, or
  // when other provided indices are invalid.
  class OutOfRangeException : public Exception {
    using Exception::Exception;
  };

  // Codepoints is an iterator over the codepoints of a String. Codepoints is
  // also used as a cursor into a String; because utf8 strings are not generally
  // byte indexable, Codepoints can be used to refer to specific codepoint
  // boundaries inside a String at which a codepoint is known to start or end.
  //
  // Because Codepoints is also an iterator, it can have an end limit,
  // specifically when constructed from a View.
  class Codepoints {
  public:
    // string is the String that this Codepoints reads from.
    const String& string;

    Codepoints(const Codepoints&) = default;

    // Creates a Codepoints that iterates through all codepoints of the provided
    // String.
    Codepoints(
        const String& string)
        : string {string},
          end_ {string.data_.size() - 1} {}

    // peek returns the next utf8 codepoint, if there is one.
    OptionalCodepoint peek() const {
      if (index_ >= end_) {
        return OptionalCodepoint {};
      }

      auto     s       = string.data_.data() + index_;
      SizeType advance = 0;

      Codepoint cp = codepoint(advance, s);
      if (advance == 0) {
        return OptionalCodepoint {};
      }

      return OptionalCodepoint {cp};
    }

    // next returns the next utf8 codepoint, if there is one, and increments the
    // iterator.
    OptionalCodepoint next() {
      if (index_ >= end_) {
        return OptionalCodepoint {};
      }

      auto     s       = string.data_.data() + index_;
      SizeType advance = 0;

      Codepoint cp = codepoint(advance, s);
      if (advance == 0) {
        index_ = string.data_.size();
        return OptionalCodepoint {};
      }

      index_ += advance;

      return OptionalCodepoint {cp};
    }

    // advances iterates this Codepoints count times.
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

    // move_to moves this Codepoints to the position pointed to by the provided
    // iterator. The provided iterator must be from the same string. The
    // endpoint of this Codepoints is not modified.
    Codepoints& move_to(
        const Codepoints& to) {
      if (&string != &to.string) {
        throw OutOfRangeException {
            "start and end are not from the same string"};
      }

      index_ = to.index_;
      return *this;
    }

    // Comparison operators for Codepoints require that both codepoints were
    // created from the same String.
    auto operator<=>(
        const Codepoints& rhs) const {
      if (&string != &rhs.string) {
        throw OutOfRangeException {"iterators are not from the same string"};
      }

      return index_ <=> rhs.index_;
    }

    bool operator==(
        const Codepoints& rhs) const {
      if (&string != &rhs.string) {
        throw OutOfRangeException {"iterators are not from the same string"};
      }

      return index_ == rhs.index_;
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
          end_ {string.data_.size() - 1} {}

    SizeType index_ {0};
    SizeType end_;

    friend class String;
    friend class View;
  };

  // View is a view to a subset of a String.
  class View {
  public:
    const String& string;

    View(
        const String& string)
        : string {string},
          start_ {0},
          end_ {string.data_.size() - 1} {}

    View(
        const Codepoints& cp)
        : string {cp.string},
          start_ {cp.index_},
          end_ {cp.end_} {}

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

    // operator== checks if the utf8 string range referred to by this View
    // contains the same codepoints as the provided View, including the same
    // number of codepoints.
    bool operator==(
        const View& rhs) const {
      auto us {codepoints()};
      auto them {rhs.codepoints()};

      while (us && them) {
        if (us.next() != them.next()) {
          return false;
        }
      }

      return !us && !them;
    }

    struct FlaggedCodepoint {
      bool       flag {false};
      Codepoints at;

      explicit operator bool() const { return flag; }
    };

    FlaggedCodepoint starts_with(
        const View& rhs) const {
      auto us {codepoints()};
      auto them {rhs.codepoints()};

      while (us && them) {
        if (us.next() != them.next()) {
          return {
              .at = us,
          };
        }
      }

      return {.flag = true, .at = us};
    }

  private:
    SizeType start_;
    SizeType end_;

    View(
        const String&  string,
        const SizeType start)
        : string {string},
          start_ {start},
          end_ {string.data_.size() - 1} {}

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

      if (start >= string.data_.size()) {
        throw OutOfRangeException {"invalid start"};
      }

      if (end >= string.data_.size()) {
        throw OutOfRangeException {"invalid end"};
      }
    }

    friend class Codepoints;
    friend class String;
  };

public:
  String() : data_ {{'\0'}} {}

  String(
      const Codepoints& start,
      const Codepoints& end) {
    if (&start.string != &end.string) {
      throw OutOfRangeException {"start and end are not from the same string"};
    }

    data_ = Vector(
        start.string.data_.data() + start.index_,
        start.string.data_.data() + end.index_);

    normalize_();
  }

  String(
      Vector&& codepoints)
      : data_ {Traits::move_vector(codepoints)} {
    normalize_();
  }

  template <typename OtherTraits>
  String(
      const String<OtherTraits>& rhs)
      : data_ {rhs.data_} {}

  template <typename OtherTraits>
  String(
      String<OtherTraits>&& rhs)
      : data_ {Traits::move_vector(rhs.data_)} {}

  bool operator<=>(
      const String& rhs) const {
    return view() <=> rhs.view();
  }

  bool operator==(
      const String& rhs) const {
    return view() == rhs.view();
  }

  template <typename OtherTraits>
  String& operator=(
      const String<OtherTraits>& rhs) {
    data_ = rhs.data_;
    return *this;
  }

  template <typename OtherTraits>
  String& operator=(
      String<OtherTraits>&& rhs) {
    if (this == &rhs) {
      return *this;
    }

    data_ = Traits::move_vector(rhs.data_);
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

  // c_str is included for compatibility with methods that expect C-style null
  // terminated strings.
  const char* c_str() const {
    return reinterpret_cast<const char*>(data_.data());
  }

private:
  Vector data_ {};

  void normalize_() {
    if (data_.back() != '\0') {
      data_.push_back('\0');
    }
  }

  String(
      const char* start,
      const char* end)
      : data_ {
            start,
            end} {
    normalize_();
  }
};

} // namespace utf8::string
