#pragma once

#include <sol/sol.hpp>

struct ScriptComponent {
  sol::function function;

  ScriptComponent(sol::function func = sol::lua_nil): function(func) {};
};
