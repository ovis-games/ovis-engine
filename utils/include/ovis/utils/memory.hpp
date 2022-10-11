#pragma once

#include <cstdint>
namespace ovis {

inline void* OffsetAddress(void* address, std::uintptr_t offset) {
  return reinterpret_cast<uint8_t*>(address) + offset;
}

inline const void* OffsetAddress(const void* address, std::uintptr_t offset) {
  return reinterpret_cast<const uint8_t*>(address) + offset;
}

}  // namespace ovis
