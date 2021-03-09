#pragma once

#include <lua.h>

namespace ovis {

int LoadMathModule(lua_State* l);
int LoadVector2Module(lua_State* l);
int LoadVector3Module(lua_State* l);
int LoadColorModule(lua_State* l);

}