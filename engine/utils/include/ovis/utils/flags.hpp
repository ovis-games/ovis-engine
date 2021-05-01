#pragma once

#include <type_traits>

namespace ovis {

template <typename EnumType>
class Flags {
  using UnderlyingType = std::underlying_type_t<EnumType>;
  template <typename T>
  friend bool operator==(Flags<T>, Flags<T>);
  template <typename T>
  friend bool operator!=(Flags<T>, Flags<T>);
  template <typename T>
  friend Flags<T> operator|(Flags<T>, Flags<T>);
  template <typename T>
  friend Flags<T>& operator|=(Flags<T>&, Flags<T>);
  template <typename T>
  friend Flags<T>& operator|=(Flags<T>&, T);
  template <typename T>
  friend Flags<T> operator&(Flags<T>, Flags<T>);

 public:
  inline Flags() : value_(0) {}
  inline Flags(EnumType value) : value_(static_cast<UnderlyingType>(value)) {}

  operator EnumType() { return static_cast<EnumType>(value_); }

  bool IsSet(Flags<EnumType> flags) const { return (value_ & flags.value_) == flags.value_; }
  bool AnySet() const { return value_ != 0; }

 private:
  UnderlyingType value_;
};

template <typename EnumType>
inline bool operator==(Flags<EnumType> lhs, Flags<EnumType> rhs) {
  return lhs.value_ == rhs.value_;
}

template <typename EnumType>
inline bool operator!=(Flags<EnumType> lhs, Flags<EnumType> rhs) {
  return lhs.value_ != rhs.value_;
}

template <typename EnumType>
inline Flags<EnumType> operator|(Flags<EnumType> lhs, Flags<EnumType> rhs) {
  return static_cast<EnumType>(lhs.value_ | rhs.value_);
}

template <typename EnumType>
inline Flags<EnumType> operator&(Flags<EnumType> lhs, Flags<EnumType> rhs) {
  return static_cast<EnumType>(lhs.value_ & rhs.value_);
}

template <typename EnumType>
inline Flags<EnumType>& operator|=(Flags<EnumType>& lhs, Flags<EnumType> rhs) {
  lhs.value_ |= rhs.value_;
  return lhs;
}

template <typename EnumType>
inline Flags<EnumType>& operator|=(Flags<EnumType>& lhs, EnumType rhs) {
  lhs.value_ |= static_cast<typename Flags<EnumType>::UnderlyingType>(rhs);
  return lhs;
}

}  // namespace ovis