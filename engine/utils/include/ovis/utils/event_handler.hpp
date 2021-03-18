#pragma once

#include <functional>
#include <vector>

#include <SDL2/SDL_assert.h>

namespace ovis {

class EventHandlerBase {
  friend class EventHandlerSubscription;

 public:
  virtual ~EventHandlerBase() = default;

 protected:
  EventHandlerBase() = default;

 private:
  virtual void Unsubscribe(std::size_t subscription_index) = 0;
};

template <class T>
class EventHandler;

class EventHandlerSubscription {
  template <typename T>
  friend class EventHandler;

 public:
  inline void Unsubscribe() {
    event_->Unsubscribe(subscription_index_);
  }

 private:
  inline EventHandlerSubscription(EventHandlerBase* event, std::size_t subscription_index)
      : event_(event), subscription_index_(subscription_index) {}

  EventHandlerBase* event_;
  std::size_t subscription_index_;
};

template <typename... ArgumentTypes>
class EventHandler<void(ArgumentTypes...)> : public EventHandlerBase {
 public:
  using FunctionType = std::function<void(ArgumentTypes...)>;

  EventHandlerSubscription Subscribe(const FunctionType& function) {
    std::size_t subscription_index = 0;
    while (subscription_index < subscriptions_.size()) {
      if (!subscriptions_[subscription_index]) {
        subscriptions_[subscription_index] = function;
        return {this, subscription_index};
      } else {
        ++subscription_index;
      }
    }

    subscriptions_.push_back(function);
    return {this, subscriptions_.size() - 1};
  }

  void Unsubscribe(std::size_t subscription_index) override {
    SDL_assert(subscription_index < subscriptions_.size());
    SDL_assert(subscriptions_[subscription_index]);
    subscriptions_[subscription_index] = nullptr;
  }

  void Invoke(ArgumentTypes&&... arguments) {
    for (const auto& subscription : subscriptions_) {
      subscription(arguments...);
    }
  }

 private:
  std::vector<FunctionType> subscriptions_;
};

}  // namespace ovis