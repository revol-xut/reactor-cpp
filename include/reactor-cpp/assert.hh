/*
 * Copyright (C) 2019 TU Dresden
 * All rights reserved.
 *
 * Authors:
 *   Christian Menard
 */

#pragma once

#ifdef REACTOR_CPP_VALIDATE
#define RUNTIME_VALIDATE true
#else
#define RUNTIME_VALIDATE false
#endif

// macro for silencing unused warnings my the compiler
#define UNUSED(expr) do { (void)(expr); } while (0)

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>

namespace reactor {

class ValidationError : public std::runtime_error {
 private:
  static std::string build_message(const std::string& msg);

 public:
  ValidationError(const std::string& msg)
      : std::runtime_error(build_message(msg)) {}
};


constexpr void validate(bool condition, const std::string& message) {
      if (RUNTIME_VALIDATE and !condition) {
        throw ValidationError(message);
      }
}


}  // namespace reactor
