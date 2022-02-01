/*
 * Copyright (C) 2019 TU Dresden
 * All rights reserved.
 *
 * Authors:
 *   Christian Menard
 */

#ifndef REACTOR_CPP_TIME_HH
#define REACTOR_CPP_TIME_HH

#include "logging.hh"
#include <chrono>
#include <iostream>

namespace reactor {

using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
using Duration = std::chrono::nanoseconds;

inline auto get_physical_time() noexcept -> TimePoint { return std::chrono::system_clock::now(); }

inline namespace operators {

auto operator<<(std::ostream &out_stream, [[maybe_unused]] TimePoint tp) noexcept -> std::ostream &;

auto operator<<(std::ostream &out_stream, [[maybe_unused]] std::chrono::seconds dur) noexcept -> std::ostream &;
auto operator<<(std::ostream &out_stream, [[maybe_unused]] std::chrono::milliseconds dur) noexcept -> std::ostream &;
auto operator<<(std::ostream &out_stream, [[maybe_unused]] std::chrono::microseconds dur) noexcept -> std::ostream &;
auto operator<<(std::ostream & out_stream, [[maybe_unused]] std::chrono::nanoseconds dur) noexcept -> std::ostream &;

} // namespace operators

} // namespace reactor

#endif // REACTOR_CPP_TIME_HH
