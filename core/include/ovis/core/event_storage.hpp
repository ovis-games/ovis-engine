#pragma once

#include "ovis/vm/list.hpp"
#include "ovis/vm/type_id.hpp"

namespace ovis {

class EventStorage {
 public:
  EventStorage(TypeId event_type_id);

  Result<> Emit(const Value& event);

 private:
  List events_;
  std::vector<bool> propagating_state_;
};

}  // namespace ovis  
