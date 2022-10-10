#pragma once

#include <cstddef>
#include <ovis/utils/type_list.hpp>

namespace ovis {
namespace reflection {

namespace detail {

template <typename T, typename M> T GetMemberPointerClass(M T::*);
template <typename T, typename M> M GetMemberPointerMember(M T::*);

template <typename T> struct Invocable;
template <typename R, typename... Args>
struct Invocable<R(*)(Args...)> {
  using ArgumentTypes = TypeList<Args...>;
  using ReturnType = R;
};
template <typename R, typename... Args>
struct Invocable<R(*)(Args...) noexcept> {
  using ArgumentTypes = TypeList<Args...>;
  using ReturnType = R;
};
template <typename C, typename R, typename... Args>
struct Invocable<R(C::*)(Args...)> {
  using ArgumentTypes = TypeList<C*, Args...>;
  using ReturnType = R;
};
template <typename C, typename R, typename... Args>
struct Invocable<R(C::*)(Args...) noexcept> {
  using ArgumentTypes = TypeList<C*, Args...>;
  using ReturnType = R;
};
template <typename C, typename R, typename... Args>
struct Invocable<R(C::*)(Args...) const> {
  using ArgumentTypes = TypeList<const C*, Args...>;
  using ReturnType = R;
};
template <typename C, typename R, typename... Args>
struct Invocable<R(C::*)(Args...) const noexcept> {
  using ArgumentTypes = TypeList<const C*, Args...>;
  using ReturnType = R;
};

}  // namespace detail

template <auto FUNCTION>
struct Invocable : public detail::Invocable<decltype(FUNCTION)> {};

template <auto MEMBER_POINTER>
struct MemberPointer {
  // See: https://stackoverflow.com/questions/12811330/c-compile-time-offsetof-inside-a-template
  using ClassType = decltype(detail::GetMemberPointerClass(MEMBER_POINTER));
  using MemberType = decltype(detail::GetMemberPointerMember(MEMBER_POINTER));
  static const std::size_t offset = reinterpret_cast<std::size_t>(&(((ClassType*)0)->*MEMBER_POINTER));
};

}  // namespace reflection
}  // namespace ovis
