#pragma once

#include <string>
#include <unordered_set>

#include "ovis/utils/result.hpp"
#include "ovis/vm/type_id.hpp"

namespace ovis {

template <typename PrepareParameters, typename ExecuteParameters>
class Job {
 public:
  Job(std::string_view id) : id_(id) {}
  virtual ~Job() = default;

  std::string_view id() const { return id_; }

  const std::unordered_set<TypeId>& read_access() const {
    return read_access_;
  }
  const std::unordered_set<TypeId>& write_access() const {
    return write_access_;
  }

  const std::unordered_set<std::string>& execute_after() const {
    return execute_after_;
  }

  const std::unordered_set<std::string>& execute_before() const {
    return execute_before_;
  }

  virtual Result<> Prepare(const PrepareParameters& parameters) = 0;
  virtual Result<> Execute(const ExecuteParameters& parameters) = 0;

 protected:
  void RequireReadAccess(TypeId resource_type) { read_access_.insert(resource_type); }
  void RequireWriteAccess(TypeId resource_type) { write_access_.insert(resource_type); }

  void ExecuteAfter(std::string_view job_id) { execute_after_.emplace(job_id); }
  void ExecuteBefore(std::string_view job_id) { execute_after_.emplace(job_id); }

 private:
  std::string id_;
  std::unordered_set<std::string> execute_after_;
  std::unordered_set<std::string> execute_before_;

  std::unordered_set<TypeId> read_access_;
  std::unordered_set<TypeId> write_access_;
};

}  // namespace ovis
