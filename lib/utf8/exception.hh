#pragma once

#include "traits.hh"

namespace utf8::string {

// Exception is the base exception that all String exceptions inherit from.
template <typename Traits> class Exception : public Traits::BaseException {
public:
  Exception(
      const char* what)
      : what_ {what} {}

  virtual const char* what() const noexcept override { return what_; }

private:
  const char* what_ {nullptr};
};

} // namespace utf8::string
