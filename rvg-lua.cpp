// Stroke-to-fill conversion program and test harness
// Copyright (C) 2020 Diego Nehab
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// Contact information: diego.nehab@gmail.com
//
#include <utility> // std::forward, std::make_index_sequence
#include <boost/iterator/iterator_facade.hpp>

#include "rvg-lua.h"

void rvg_lua_checknargs(lua_State *L, int n) {
    int t = lua_gettop(L);
    if (t > n) {
        luaL_argerror(L, t, "too many arguments");
    } else if (t < n) {
        luaL_argerror(L, n, "too few arguments");
    }
}

void rvg_lua_require(lua_State *L, const char* modname) {
    lua_getglobal(L, "require");
    lua_pushstring(L, modname);
    lua_call(L, 1, 1);
}

void rvg_lua_setuservalue(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    lua_setuservalue(L, idx);
#else
    lua_setfenv(L, idx);
#endif
}

void rvg_lua_getuservalue(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    lua_getuservalue(L, idx);
#else
    lua_getfenv(L, idx);
#endif
}

int rvg_lua_abs_index(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    return lua_absindex(L, idx);
#else
    // if not a pseudo index, convert to absolute
    if (idx <= LUA_REGISTRYINDEX) return idx;
    else return idx < 0? lua_gettop(L) + idx + 1: idx;
#endif
}

FILE* rvg_lua_check_file(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    luaL_Stream *ls = (luaL_Stream *) luaL_checkudata(L, idx, LUA_FILEHANDLE);
    if (ls->closef == NULL) luaL_argerror(L, idx, "file is closed");
    return ls->f;
#else
    FILE **fpp=(FILE **)luaL_checkudata(L, idx, LUA_FILEHANDLE);
    if (!*fpp) luaL_argerror(L, idx, "file is closed");
    return *fpp;
#endif
}

int rvg_lua_is_equal(lua_State *L, int idx1, int idx2) {
#if LUA_VERSION_NUM > 501
    return lua_compare(L, idx1, idx2, LUA_OPEQ);
#else
    return lua_equal(L, idx1, idx2);
#endif
}

int rvg_lua_len(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    return static_cast<int>(luaL_len(L, idx));
#else
    if (lua_type(L, idx) == LUA_TUSERDATA) {
        int top = lua_gettop(L);
        if (!luaL_callmeta(L, idx, "__len")) {
            luaL_error(L, "attempt to get length of userdata value");
        }
        if (!lua_isnumber(L, -1)) {
            luaL_error(L, "object length is not a number");
        }
        int n = lua_tointeger(L, -1);
        lua_settop(L, top);
        return n;
    } else
        return static_cast<int>(lua_objlen(L, idx));
#endif
}

void rvg_lua_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
#if LUA_VERSION_NUM < 502
  // copied from Lua 5.2
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
#else
  return luaL_setfuncs(L, l, nup);
#endif
}

float rvg_lua_checkfloat(lua_State *L, int idx) {
	return static_cast<float>(luaL_checknumber(L, idx));
}

float rvg_lua_optfloat(lua_State *L, int idx, float def) {
	return static_cast<float>(luaL_optnumber(L, idx, def));
}

int rvg_lua_optboolean(lua_State *L, int idx, int def) {
	if (lua_isnoneornil(L, idx)) {
		return def;
	} else if (!lua_isboolean(L, idx)) {
		luaL_argerror(L, idx, "expected boolean");
		return 0; // never reached
	} else {
		return lua_toboolean(L, idx);
	}
}

float rvg_lua_tofloat(lua_State *L, int idx) {
	return static_cast<float>(lua_tonumber(L, idx));
}

template <>
int rvg_lua_push<float>(lua_State *L, const float &value, int) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
int rvg_lua_push<int32_t>(lua_State *L, const int32_t &value, int) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
int rvg_lua_push<double>(lua_State *L, const double &value, int) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
int rvg_lua_push<int64_t>(lua_State *L, const int64_t &value, int) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
int rvg_lua_push<double>(lua_State *L, double &&value, int) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
int rvg_lua_push<int64_t>(lua_State *L, int64_t &&value, int) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
int rvg_lua_push<float>(lua_State *L, float &&value, int) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
int rvg_lua_push<int32_t>(lua_State *L, int32_t &&value, int) {
	lua_pushinteger(L, value);
	return 1;
}


template <>
int rvg_lua_check<int32_t>(lua_State *L, int idx, int) {
    return static_cast<int32_t>(luaL_checkinteger(L, idx));
}

template <>
float rvg_lua_check<float>(lua_State *L, int idx, int) {
    return static_cast<float>(luaL_checknumber(L, idx));
}

template <>
double rvg_lua_check<double>(lua_State *L, int idx, int) {
    return static_cast<double>(luaL_checknumber(L, idx));
}

template <>
int rvg_lua_push<const char *>(lua_State *L, const char * const &value, int) {
	lua_pushstring(L, value);
	return 1;
}

template <>
int rvg_lua_push<const char *>(lua_State *L, const char * &&value, int) {
	lua_pushstring(L, value);
	return 1;
}

void rvg_lua_print(lua_State *L, int idx) {
    idx = rvg_lua_abs_index(L, idx);
    lua_getglobal(L, "tostring");
    lua_pushvalue(L, idx);
    lua_call(L, 1, 1);
    fprintf(stderr, "%02d: %s\n", idx, lua_tostring(L, -1));
    lua_pop(L, 1);
}

int rvg_lua_geti(lua_State *L, int idx, int n) {
#if LUA_VERSION_NUM >= 503
    return lua_geti(L, idx, n);
#else
    idx = rvg_lua_abs_index(L, idx);
    lua_pushnumber(L, n);
    lua_gettable(L, idx);
    return lua_type(L, -1);
#endif
}

void rvg_lua_seti(lua_State *L, int idx, int n) {
#if LUA_VERSION_NUM >= 503
    lua_seti(L, idx, n);
#else
    idx = rvg_lua_abs_index(L, idx);
    lua_pushnumber(L, n);
    lua_insert(L, -2);
    lua_settable(L, idx);
#endif
}

// copied from Lua 5.3.4
static int unpack(lua_State *L) {
  size_t n;
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, rvg_lua_len(L, 1));
  if (i > e) return 0;  /* empty range */
  n = (size_t)(e - i);  /* number of elements minus 1 (avoid overflows) */
  if (n >= (unsigned int)INT_MAX  || !lua_checkstack(L, (int)(++n)))
    return luaL_error(L, "too many results to unpack");
  for (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    rvg_lua_geti(L, 1, static_cast<int>(i));
  }
  rvg_lua_geti(L, 1, static_cast<int>(e));  /* push last element */
  return (int)n;
}

// copied from Lua 5.3.4
static int ipairsaux (lua_State *L) {
    lua_Integer i = luaL_checkinteger(L, 2) + 1;
    lua_pushinteger(L, i);
    return (rvg_lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}

// copied Lua 5.3.4
static int ipairs (lua_State *L) {
    luaL_checkany(L, 1);
    lua_pushcfunction(L, ipairsaux);  /* iteration function */
    lua_pushvalue(L, 1);  /* state */
    lua_pushinteger(L, 0);  /* initial value */
    return 3;
}

int rvg_lua_init(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, RVG_LUA_CONTEXT);
    if (lua_isnil(L, -1)) { // nil
        lua_pop(L, 1); //
        lua_newtable(L); // ctxtab
        lua_pushvalue(L, -1); // ctxtab ctxtab
        lua_setfield(L, LUA_REGISTRYINDEX, RVG_LUA_CONTEXT); // ctxtab
#if LUA_VERSION_NUM >= 503
        lua_getglobal(L, "math");
        lua_getfield(L, -1, "atan");
        lua_setfield(L, -2, "atan2");
        lua_pop(L, 1);
#endif
        lua_getglobal(L, "table");
        lua_pushcfunction(L, unpack);
        lua_setfield(L, -2, "unpack");
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_setglobal(L, "unpack");
        lua_pushcfunction(L, ipairs);
        lua_setglobal(L, "ipairs");
    }
    // ctxtab
    return 1;
}

std::vector<std::string> rvg_lua_checkargs(lua_State *L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    int n = rvg_lua_len(L, idx);
    auto get = [](lua_State *L, int tab, int item) {
        lua_rawgeti(L, tab, item);
        if (!lua_isstring(L, -1))
            luaL_error(L, "entry %d is not a string", item);
        std::string str(lua_tostring(L, -1));
        lua_pop(L, 1);
        return str;
    };
    auto range = rvg_lua_make_table_range<std::string>(L, idx, 1, n+1, get);
    std::vector<std::string> args;
    args.insert(args.begin(), range.first, range.second);
    return args;
}

std::vector<std::string> rvg_lua_optargs(lua_State *L, int idx,
    const std::vector<std::string> &def) {
    if (!lua_istable(L, idx)) return def;
    else return rvg_lua_checkargs(L, idx);
}

std::vector<float> rvg_lua_checkfloatvector(lua_State *L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    int n = rvg_lua_len(L, idx);
    auto get = [](lua_State *L, int tab, int item) {
        lua_rawgeti(L, tab, item);
        if (!lua_isnumber(L, -1))
            luaL_error(L, "entry %d is not a number", item);
        float f = rvg_lua_tofloat(L, -1);
        lua_pop(L, 1);
        return f;
    };
    auto range = rvg_lua_make_table_range<float>(L, idx, 1, n+1, get);
	std::vector<float> floats;
    floats.insert(floats.begin(), range.first, range.second);
	return floats;
}

static int proxynewindexdenied(lua_State *L) {
    luaL_error(L, "table is read-only");
    return 0;
}

static int proxynext(lua_State *L) {
    while (1) {
        if (lua_next(L, -2)) {
            if (lua_type(L, -2) == LUA_TSTRING) {
                break;
            } else {
                lua_pop(L, 1);
            }
        } else {
            lua_pushnil(L);
            lua_pushnil(L);
            break;
        }
    }
    return 2;
}

static int proxypairsoverindex(lua_State *L) {
    lua_pushcfunction(L, proxynext);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushnil(L);
    return 3;
}

static int proxyindex(lua_State *L) {
    lua_pushvalue(L, -1);
    lua_gettable(L, lua_upvalueindex(1));
    if (lua_isnil(L, -1))
        luaL_error(L, "%s not enum member", lua_tostring(L, -2));
    return 1;
}

void rvg_lua_readonlyproxy(lua_State *L) {
    // srctab
    lua_newtable(L); // srctab proxytab
    lua_newtable(L); // srctab proxytab proxytabmeta
    lua_pushvalue(L, -3); // srctab proxytab proxytabmeta srctab
    lua_pushcclosure(L, proxyindex, 1);
    lua_setfield(L, -2, "__index"); // srctab proxytab proxytabmeta
    lua_pushcfunction(L, proxynewindexdenied); // srctab proxytab proxytabmeta newindex
    lua_setfield(L, -2, "__newindex"); // srctab proxytab proxytabmeta
    lua_pushvalue(L, -3); // srctab proxytab proxytabmeta srctab
    lua_pushcclosure(L, proxypairsoverindex, 1);
    lua_setfield(L, -2, "__pairs"); // srctab proxytab proxytabmeta
    lua_setmetatable(L, -2); // srctab proxytab
    lua_remove(L, -2); // proxytab
}
