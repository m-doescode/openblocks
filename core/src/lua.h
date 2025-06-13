#pragma once
extern "C" {
    #include <luajit.h>
    #include <lauxlib.h>
    #include <lualib.h>
    #include <lua.h>
}

inline const char* x_luaL_udatatname (lua_State *L, int ud) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {
    lua_getmetatable(L, ud);
    lua_getfield(L, -1, "__name");
    const char* str = lua_tostring(L, -1);
    lua_pop(L, 2);
    return str;
  }
  return NULL;
}