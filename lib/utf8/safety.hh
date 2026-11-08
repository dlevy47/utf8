#pragma once

#include "exception.hh"

namespace utf8::string {

template <typename Traits> struct ChecksEnabled {
  using BaseException = typename Traits::BaseException;
  using Exception     = typename ::utf8::string::Exception<Traits>;

  // CheckException is thrown when an operation whose arguments are
  // required to point to the same object is attempted with arguments
  // that don't point to the same object.
  class CheckException : public Exception {
    using Exception::Exception;
  };

  template <typename T>
  static void check_same_object(
      const T* t1,
      const T* t2) {
    if (t1 != t2) {
      throw CheckException {"string objects are not the same"};
    }
  }
};

struct ChecksDisabled {
  template <typename T>
  static void check_same_object(
      const T* t1,
      const T* t2) {}
};

} // namespace utf8::string
