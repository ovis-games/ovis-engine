#pragma once

#include <cstdint>

#include <ovis/utils/versioned_index.hpp>

namespace ovis {

using TypeId = VersionedIndex<uint32_t, 20>;

}
