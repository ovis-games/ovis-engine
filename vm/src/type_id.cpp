#include "ovis/vm/type_id.hpp"

#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

TypeId GetTypeIdFromNativeTypeId(NativeTypeId native_type_id, VirtualMachine* virtual_machine) {
  return virtual_machine->GetTypeId(native_type_id);
}

}  // namespace ovis
