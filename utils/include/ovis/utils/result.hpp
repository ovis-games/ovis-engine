#pragma once

#include "ovis/utils/log.hpp"
#include <cassert>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <type_traits>
#include <optional>

namespace ovis {

struct Error {
  template <typename... Args>
  Error(std::string_view message, Args&&... args) : message(fmt::format(fmt::runtime(message), std::forward<Args>(args)...)) {}

  std::string message;
};

template <typename T = void, typename E = Error>
class Result;

struct SuccessType {};
constexpr SuccessType Success;

template <typename E>
class Result<void, E> {
public:
  [[deprecated]] Result() {}
  Result(SuccessType) {}
  Result(E error) : error_(std::move(error)) {}

  [[nodiscard]] operator bool() const {
#ifndef NDEBUG
    has_been_checked_ = true;
#endif
    return !error_;
  }

  const E &error() const {
    assert(has_been_checked_);
    return *error_;
  }

private:
  std::optional<E> error_;
#ifndef NDEBUG
  mutable bool has_been_checked_ = false;
#endif
};

template <typename T, typename E>
class Result {
public:
  Result(T value) : has_value_(true) {
    new (&data_) T(std::move(value));
  }
  Result(E error) : has_value_(false) {
    new (&data_) E(std::move(error));
  }

  Result(const Result& result) = delete;
  Result(Result&& result) = delete;

  ~Result() {
    if (has_value_) {
      data_as_value()->~T();
    } else {
      data_as_error()->~E();
    }
  }

  Result& operator=(const Result& result) = delete;
  Result& operator=(Result&& result) = delete;

  [[nodiscard]] bool has_value() const {
#ifndef NDEBUG
    has_been_checked_ = true;
#endif
    return has_value_;
  }

  [[nodiscard]] operator bool() const {
    return has_value();
  }

  const E& error() const {
    assert(!has_value_);
    assert(has_been_checked_);
    return *data_as_error();
  }

  T* operator->() {
    assert(has_value_);
    assert(has_been_checked_);
    return data_as_value();
  }
  const T* operator->() const {
    assert(has_value_);
    assert(has_been_checked_);
    return data_as_value();
  }

  T& operator*() {
    assert(has_value_);
    assert(has_been_checked_);
    return *data_as_value();
  }
  const T& operator*() const {
    assert(has_value_);
    assert(has_been_checked_);
    return *data_as_value();
  }

private:
  std::aligned_union_t<0, T, E> data_;
  bool has_value_;
#ifndef NDEBUG
  mutable bool has_been_checked_ = false;
#endif
  static_assert(!std::is_constructible_v<T, E>);

  T* data_as_value() {
    return reinterpret_cast<T*>(&data_);
  }
  const T* data_as_value() const {
    return reinterpret_cast<const T*>(&data_);
  }

  E* data_as_error() {
    return reinterpret_cast<E*>(&data_);
  }
  const E* data_as_error() const {
    return reinterpret_cast<const E*>(&data_);
  }
};

template <typename T, typename ResultType, typename E>
bool operator==(const Result<ResultType, E>& lhs, const T& rhs) {
  return lhs.has_value() && *lhs == rhs;
}

template <typename T, typename E>
bool operator==(const T& lhs, const Result<T, E>& rhs) {
  return rhs.has_value() && lhs == *rhs;
}

template <typename T>
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<Result<T, E>> : std::true_type {};

template <typename T>
constexpr bool is_result_v = is_result<T>::value;

template <typename T>
concept ResultType = is_result_v<T>;

template <typename T>
concept NonResultType = !is_result_v<T>;

template <typename T, typename E>
requires std::is_base_of_v<Error, E> void LogOnError(const Result<T, E>& result, LogLevel log_level = LogLevel::ERROR) {
  if (!result) {
    Log::Write(log_level, "{}", result.error().message);
  }
}

#define OVIS_CHECK_RESULT(expression)                              \
  if (auto&& ovis_check_result = expression; !ovis_check_result) { \
    return ovis_check_result.error();                              \
  }

}  // namespace ovis


template<>
struct fmt::formatter<ovis::Error> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const ovis::Error& error, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "{}", error.message);
  }
};

