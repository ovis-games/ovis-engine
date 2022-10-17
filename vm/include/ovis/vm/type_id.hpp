#pragma once

#include <cstdint>

#include "ovis/utils/native_type_id.hpp"
#include "ovis/utils/versioned_index.hpp"

namespace ovis {

class VirtualMachine;

using TypeId = VersionedIndex<uint32_t, 20>;

TypeId GetTypeIdFromNativeTypeId(NativeTypeId native_type_id, VirtualMachine* virtual_machine);

}  // namespace ovis
