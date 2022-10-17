#include "ovis/core/event_storage.hpp"
#include "ovis/core/main_vm.hpp"

namespace ovis {

EventStorage::EventStorage(TypeId event_type_id) : events_(event_type_id, main_vm) {}

}  // namespace ovis
