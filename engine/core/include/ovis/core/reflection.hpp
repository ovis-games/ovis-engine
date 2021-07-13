#pragma once

#include <string_view>
#include <string>

namespace ovis {

class Property {

};

class Type {
public:
  Type(std::string_view name, Type* base = nullptr);

  std::string_view name() const {
    return name_;
  }

private:
  std::string name_;
};

}  // namespace ovis
