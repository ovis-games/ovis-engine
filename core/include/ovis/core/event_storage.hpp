#pragma once

#include "ovis/utils/range.hpp"
#include "ovis/vm/list.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/core/main_vm.hpp"

namespace ovis {

class EventStorage {
 public:
  EventStorage(TypeId event_type_id);

  TypeId event_type_id() const { return events_.element_type(); }

  Result<> Emit(const Value& event) {
    OVIS_CHECK_RESULT(events_.Add(event));
    propagating_state_.push_back(true);
    return Success;
  }

  void Clear() {
    events_.Resize(0);
    propagating_state_.clear();
  }

  auto size() const { return events_.size(); }

  template <typename T>
  const T& Get(ContiguousStorage::SizeType index) const {
    return events_.Get<T>(index);
  }

  bool IsEventPropagating(ContiguousStorage::SizeType index) const {
    return propagating_state_[index];
  }

  void StopPropagating(ContiguousStorage::SizeType index) {
    propagating_state_[index] = false;
  }

 private:
  List events_;
  std::vector<bool> propagating_state_;
};

template <typename T>
class EventEmitter {
 public:
  EventEmitter(EventStorage* event_storage = nullptr) : event_storage_(event_storage) {
    assert(!event_storage_ || event_storage_->event_type_id() == main_vm->GetTypeId<T>());
  }

  Result<> Emit(const T& event) { return event_storage_->Emit(main_vm->CreateValue(event)); }

 private:
  EventStorage* event_storage_;
};

template <typename T>
class Event {
  public:
   Event(EventStorage* event_storage, ContiguousStorage::SizeType index)
       : event_storage_(event_storage), index_(index) {}

  const T& operator*() const { return event_storage_->Get<T>(index_); }
  const T* operator->() { return &event_storage_->Get<T>(index_); }

  bool is_propagating() const { return event_storage_->IsEventPropagating(index_); }

  void StopPropagating() { event_storage_->StopPropagating(index_); }

  auto index() const { return index_; }
  auto event_storage() const { return event_storage_; }

 private:
  EventStorage* event_storage_;
  ContiguousStorage::SizeType index_;
};

template <typename T>
class EventStorageView {
 public:
  EventStorageView(EventStorage* event_storage = nullptr) : event_storage_(event_storage) {
    assert(!event_storage_ || event_storage_->event_type_id() == main_vm->GetTypeId<T>());
  }

  TypeId event_type_id() const { event_storage_->event_type_id(); }
  auto size() const { return event_storage_->size(); }

  Result<> Emit(const T& event) { return event_storage_->Emit(main_vm->CreateValue(event)); }
  void Clear() { event_storage_->Clear(); }

  Event<T> operator[](ContiguousStorage::SizeType index) { return {event_storage_, index}; }
  Event<T> At(ContiguousStorage::SizeType index) const { return {event_storage_, index}; }

  class Iterator {
    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.event_.event_storage() == rhs.event_.event_storage() && lhs.event_.index() == rhs.event_.index();
    }
    friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
      return lhs.event_.event_storage() != rhs.event_.event_storage() || lhs.event_.index() != rhs.event_.index();
    }

   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Event<T>;
    using pointer = Event<T>*;
    using reference = Event<T>&;

    Iterator() : event_(nullptr, 0) {}
    Iterator(EventStorage* event_storage, ContiguousStorage::SizeType index) : event_(event_storage, index) {}

    Event<T> operator*() const { return event_; }
    pointer operator->() { return &event_; }
    Iterator& operator++() {
      Increment();
      return *this;
    }
    Iterator operator++(int) {
      auto current = *this;
      Increment();
      return current;
    }

   private:
    void Increment() {
      auto storage = event_.event_storage();
      const ContiguousStorage::SizeType storage_size = storage->size();
      ContiguousStorage::SizeType new_index = event_.index();
      do {
        ++new_index;
      } while (!storage->IsEventPropagating(new_index) && new_index < storage_size);
      event_ = Event<T>(storage, new_index);
    }
    Event<T> event_;
  };
  static_assert(std::forward_iterator<Iterator>);
  
  Iterator begin() {
    ContiguousStorage::SizeType index = 0;
    while (!event_storage_->IsEventPropagating(index) && index < event_storage_->size()) {
      ++index;
    }
    return Iterator(event_storage_, index);
  }
  Iterator end() { return Iterator(event_storage_, event_storage_->size()); }

 private:
  EventStorage* event_storage_;
};

}  // namespace ovis  
