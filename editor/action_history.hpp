#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <fmt/core.h>

#include <ovis/core/json.hpp>
#include <ovis/core/log.hpp>

namespace ove {

struct Action {
  std::string id;
  ovis::json data;
  std::string description;
};

class ActionHistoryBase {
 public:
  ActionHistoryBase() : current_position_(actions_.end()) {}
  virtual ~ActionHistoryBase() = default;

  virtual bool Undo() = 0;
  virtual bool Redo() = 0;

  inline bool undo_possible() const { return current_position_ != actions_.begin(); }
  inline bool redo_possible() const { return current_position_ != actions_.end(); }

  inline std::string undo_description() const {
    return current_position_ != actions_.begin() ? (current_position_ - 1)->description : "";
  }

  inline std::string redo_description() const {
    return current_position_ != actions_.end() ? current_position_->description : "";
  }

 protected:
  std::vector<Action> actions_;
  std::vector<Action>::iterator current_position_;
};

template <typename Context>
class ActionHistory : public ActionHistoryBase {
 public:
  typedef void (*ActionFunction)(Context* context, const ovis::json& data);
  typedef std::string (*ActionDescriptionFunction)(Context* context, const ovis::json& data);

  ActionHistory(Context* context) : context_(context) {}

  void RegisterAction(const std::string& id, ActionFunction do_function, ActionFunction undo_function) {
    SDL_assert(do_functions_.count(id) == 0);
    SDL_assert(undo_functions_.count(id) == 0);

    do_functions_.insert(std::make_pair(id, do_function));
    undo_functions_.insert(std::make_pair(id, undo_function));

    SDL_assert(do_functions_.count(id) == 1);
    SDL_assert(undo_functions_.count(id) == 1);
  }

  template <typename... DesciptionArguments>
  void Do(const std::string& action_id, const ovis::json& data, const char* description,
          DesciptionArguments&&... description_arguments) {
    SDL_assert(do_functions_.count(action_id) == 1);
    do_functions_[action_id](context_, data);
    actions_.erase(current_position_, actions_.end());
    actions_.push_back(
        Action{action_id, data, fmt::format(description, std::forward<DesciptionArguments>(description_arguments)...)});
    current_position_ = actions_.end();
    ovis::LogV("{} (action_id={}, data={})", actions_.back().description, action_id, data.dump());
  }

  bool Redo() override {
    if (current_position_ == actions_.end()) {
      return false;
    }
    do_functions_[current_position_->id](context_, current_position_->data);
    ovis::LogV("Redo: {} (action_id={}, data={})", current_position_->description, current_position_->id,
               current_position_->data.dump());
    ++current_position_;
    return true;
  }

  bool Undo() override {
    if (current_position_ == actions_.begin()) {
      return false;
    }
    --current_position_;
    undo_functions_[current_position_->id](context_, current_position_->data);
    ovis::LogV("Undo: {} (action_id={}, data={})", current_position_->description, current_position_->id,
               current_position_->data.dump());
    return true;
  }

 private:
  Context* context_;

  std::unordered_map<std::string, ActionFunction> do_functions_;
  std::unordered_map<std::string, ActionFunction> undo_functions_;
};

}  // namespace ove