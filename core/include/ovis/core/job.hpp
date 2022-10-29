#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ovis/utils/result.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/core/main_vm.hpp"
#include "ovis/core/resource.hpp"

namespace ovis {

template <typename PrepareParameters, typename ExecuteParameters>
class Job {
 public:
  Job(std::string_view id) : id_(id) {}
  virtual ~Job() = default;

  std::string_view id() const { return id_; }

  const auto& required_resources() const { return required_resources_; }

  const std::unordered_set<std::string>& execute_after() const { return execute_after_; }

  const std::unordered_set<std::string>& execute_before() const { return execute_before_; }

  virtual Result<> Prepare(const PrepareParameters& parameters) = 0;
  virtual Result<> Execute(const ExecuteParameters& parameters) = 0;

 protected:
  template <typename T>
  void RequireResourceAccess(ResourceAccess access_type) {
    RequireResourceAccess(main_vm->GetTypeId<T>(), access_type);
  }

  void RequireResourceAccess(TypeId resource_type, ResourceAccess access_type) {
    required_resources_[resource_type] = access_type;
  }

  void ExecuteAfter(std::string_view job_id) { execute_after_.emplace(job_id); }
  void ExecuteBefore(std::string_view job_id) { execute_after_.emplace(job_id); }

 private:
  std::string id_;
  std::unordered_set<std::string> execute_after_;
  std::unordered_set<std::string> execute_before_;

  std::unordered_map<TypeId, ResourceAccess> required_resources_;
};

}  // namespace ovis
