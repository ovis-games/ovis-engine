#pragma once

#include <sol/sol.hpp>

namespace ovis {

int LoadCoreModule(lua_State* l);
int LoadLogModule(lua_State* l);

}