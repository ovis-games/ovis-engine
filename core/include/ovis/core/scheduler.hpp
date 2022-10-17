#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "ovis/vm/list.hpp"
#include "ovis/core/event_storage.hpp"
#include "ovis/core/job.hpp"
#include "ovis/core/main_vm.hpp"

namespace ovis {

template <typename PrepareParameters, typename ExecuteParameters>
class Scheduler {
 public:
  template <typename T, typename... ConstructorArguments>
  requires (std::is_base_of_v<Job<PrepareParameters, ExecuteParameters>, T>)
  void AddJob(ConstructorArguments&&... arguments) {
    jobs_.push_back(std::make_unique<T>(std::forward<ConstructorArguments>(arguments)...));
  }

  Result<> Prepare(const PrepareParameters& parameters);
  Result<> operator()(const ExecuteParameters& parameters);

  std::unordered_set<TypeId> GetUsedComponentsAndEvents() const;
  std::unordered_set<TypeId> GetUsedEntityComponents() const;
  std::unordered_set<TypeId> GetUsedSceneComponents() const;
  std::unordered_set<TypeId> GetUsedEvents() const;

 private:
  std::vector<std::unique_ptr<Job<PrepareParameters, ExecuteParameters>>> jobs_;

  std::vector<EventStorage> event_storages_;

  Result<> SortJobs();
  Result<> SetupEventStorage();
  Result<> PrepareJobs(const PrepareParameters& parameters);
};

template <typename PrepareParameters, typename ExecuteParameters>
Result<> Scheduler<PrepareParameters, ExecuteParameters>::Prepare(const PrepareParameters& parameters) {
  OVIS_CHECK_RESULT(SortJobs());
  OVIS_CHECK_RESULT(SetupEventStorage());
  OVIS_CHECK_RESULT(PrepareJobs(parameters));
  return Success;
}

template <typename PrepareParameters, typename ExecuteParameters>
Result<> Scheduler<PrepareParameters, ExecuteParameters>::operator()(const ExecuteParameters& parameters) {
  for (const auto& job : jobs_) {
    OVIS_CHECK_RESULT(job->Execute(parameters));
  }

  return Success;
}

template <typename PrepareParameters, typename ExecuteParameters>
std::unordered_set<TypeId> Scheduler<PrepareParameters, ExecuteParameters>::GetUsedComponentsAndEvents() const {
  std::unordered_set<TypeId> types;
  for (const auto& job : jobs_) {
    types.insert(job->read_access().begin(), job->read_access().end());
    types.insert(job->write_access().begin(), job->write_access().end());
  }
  return types;
}

template <typename PrepareParameters, typename ExecuteParameters>
std::unordered_set<TypeId> Scheduler<PrepareParameters, ExecuteParameters>::GetUsedEntityComponents() const {
  auto components = GetUsedComponentsAndEvents();
  std::erase_if(components, [](TypeId type_id) {
    return !main_vm->GetType(type_id)->attributes().contains("Core.EntityComponent");
  });
  return components;
}

template <typename PrepareParameters, typename ExecuteParameters>
std::unordered_set<TypeId> Scheduler<PrepareParameters, ExecuteParameters>::GetUsedSceneComponents() const {
  auto components = GetUsedComponentsAndEvents();
  std::erase_if(components, [](TypeId type_id) {
    return !main_vm->GetType(type_id)->attributes().contains("Core.SceneComponent");
  });
  return components;
}

template <typename PrepareParameters, typename ExecuteParameters>
std::unordered_set<TypeId> Scheduler<PrepareParameters, ExecuteParameters>::GetUsedEvents() const {
  auto components = GetUsedComponentsAndEvents();
  std::erase_if(components, [](TypeId type_id) {
    return !main_vm->GetType(type_id)->attributes().contains("Core.Event");
  });
  return components;
}

template <typename PrepareParameters, typename ExecuteParameters>
Result<> Scheduler<PrepareParameters, ExecuteParameters>::SortJobs() {
  std::unordered_multimap<std::string_view, std::string_view> dependencies;

  for (const auto& job : jobs_) {
    for (const auto& execute_after : job->execute_after()) {
      dependencies.insert(std::make_pair(job->id(), execute_after));
    }
    for (const auto& execute_before : job->execute_before()) {
      dependencies.insert(std::make_pair(execute_before, job->id()));
    }
  }

  // The range [jobs.begin(), jobs.begin() + processed_jobs) is alrady sorted
  // The range [jobs.begin() + processed_jobs, jobs.end()) still needs to be sorted
  for (size_t processed_jobs = 0; processed_jobs < jobs_.size(); ++processed_jobs) {
    auto unsorted_section_start = jobs_.begin() + processed_jobs;
    auto next_job = std::find_if(unsorted_section_start, jobs_.end(),
                                 [&](const auto& job) { return dependencies.count(job->id()) == 0; });
    if (next_job == jobs_.end()) {
      return Error("Job dependencies cannot be resolved");
    }

    std::string_view next_job_id = next_job->get()->id();
    std::erase_if(dependencies, [next_job_id](const auto& dependency) {
      return next_job_id == dependency.second;
    });

    using std::swap;
    swap(*unsorted_section_start, *next_job);
  }

  return Success;
}

template <typename PrepareParameters, typename ExecuteParameters>
Result<> Scheduler<PrepareParameters, ExecuteParameters>::SetupEventStorage() {
  const auto event_types = GetUsedEvents();
  event_storages_.clear();
  event_storages_.reserve(event_types.size());

  for (const auto event_type : event_types) {
    event_storages_.emplace_back(event_type);
  }

  return Success;
}

template <typename PrepareParameters, typename ExecuteParameters>
Result<> Scheduler<PrepareParameters, ExecuteParameters>::PrepareJobs(const PrepareParameters& parameters) {
  for (const auto& job : jobs_) {
    OVIS_CHECK_RESULT(job->Prepare(parameters));
  }
  return Success;
}

}  // namespace ovis
