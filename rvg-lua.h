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
#ifndef RVG_LUA_H
#define RVG_LUA_H

#ifndef RVG_LUAPP
extern "C" {
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifndef RVG_LUAPP
}
#endif

#include <utility>
#include <vector>
#include <string>
#include <boost/type_index.hpp>
#include <boost/iterator/iterator_facade.hpp>

void rvg_lua_require(lua_State *L, const char* modname);
void rvg_lua_require(lua_State *L, const char* modname);
FILE* rvg_lua_check_file(lua_State *L, int idx);
int rvg_lua_abs_index(lua_State *L, int idx);
int rvg_lua_is_equal(lua_State *L, int idx1, int idx2);
int rvg_lua_len(lua_State *L, int idx);
void rvg_lua_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
void rvg_lua_setuservalue(lua_State *L, int idx);
void rvg_lua_getuservalue(lua_State *L, int idx);
void rvg_lua_checknargs(lua_State *L, int n);
void rvg_lua_seti(lua_State *L, int idx, int n);
int rvg_lua_geti(lua_State *L, int idx, int n);
std::vector<std::string> rvg_lua_checkargs(lua_State *L, int idx);
std::vector<std::string> rvg_lua_optargs(lua_State *L, int idx,
    const std::vector<std::string> &def = std::vector<std::string>());
std::vector<float> rvg_lua_checkfloatvector(lua_State *L, int idx);
void rvg_lua_readonlyproxy(lua_State *L);

#define RVG_LUA_CONTEXT "rvg::lua::context"

float rvg_lua_checkfloat(lua_State *L, int idx);

float rvg_lua_optfloat(lua_State *L, int idx, float def);

int rvg_lua_optboolean(lua_State *L, int idx, int def);

float rvg_lua_tofloat(lua_State *L, int idx);

int rvg_lua_init(lua_State *L);

// helper functions
template <typename T> const char *rvg_lua_raw_name(void) {
    return boost::typeindex::type_id_with_cvr<T>().raw_name();
}

void rvg_lua_print(lua_State *L, int idx);

template <typename T>
void rvg_lua_gettypemetatable(lua_State *L, int ctxidx = lua_upvalueindex(1)) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    lua_pushstring(L, rvg_lua_raw_name<T>());
    lua_rawget(L, ctxidx);
    if (lua_isnil(L, -1))
        luaL_error(L, "unknown type (%s)",
            boost::typeindex::type_id_with_cvr<T>().pretty_name().c_str());
}

template <typename T>
int rvg_lua_typeexists(lua_State *L, int ctxidx = lua_upvalueindex(1)) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    lua_pushstring(L, rvg_lua_raw_name<T>());
    lua_rawget(L, ctxidx);
    int exists = !lua_isnil(L, -1);
    lua_pop(L, 1);
    return exists;
}

template <typename T>
int rvg_lua_is(lua_State *L, int idx, int ctxidx = lua_upvalueindex(1)) {
    idx = rvg_lua_abs_index(L, idx);
    rvg_lua_gettypemetatable<T>(L, ctxidx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    int ret = rvg_lua_is_equal(L, -1, -2);
    lua_pop(L, 2);
    return ret;
}

//??D should change to return T &
template <typename T>
T rvg_lua_to(lua_State *L, int idx) {
    return *reinterpret_cast<T *>(lua_touserdata(L, idx));
}

template <typename T>
T *rvg_lua_topointer(lua_State *L, int idx) {
    return reinterpret_cast<T *>(lua_touserdata(L, idx));
}

template <typename T>
int rvg_lua_gc(lua_State *L) {
    T *ptr = reinterpret_cast<T *>(lua_touserdata(L, 1));
    ptr->~T();
    return 0;
}

template <typename T>
int rvg_lua_tostring(lua_State *L) {
    const char *name = "unkown";
    if (luaL_getmetafield(L, 1, "name") == LUA_TSTRING) {
        name = lua_tostring(L, -1);
    }
    T *ptr = reinterpret_cast<T *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "%s: %p", name, ptr);
    return 1;
}

template <typename T>
void rvg_lua_push_name(lua_State *L, int ctxidx = lua_upvalueindex(1)) {
    rvg_lua_gettypemetatable<T>(L, ctxidx);
    lua_getfield(L, -1, "name");
    lua_replace(L, -2);
}

template <typename T>
void rvg_lua_argerror(lua_State *L, int idx, int ctxidx = lua_upvalueindex(1)) {
    idx = rvg_lua_abs_index(L, idx);
    rvg_lua_push_name<T>(L, ctxidx);
    luaL_argerror(L, idx,
        lua_pushfstring(L, "expected %s",
            lua_tostring(L, -1)));
}

template <typename T>
T* rvg_lua_check_pointer(lua_State *L, int idx, int ctxidx = lua_upvalueindex(1)) {
    if (!rvg_lua_is<T>(L, idx, ctxidx)) rvg_lua_argerror<T>(L, idx, ctxidx);
    return rvg_lua_topointer<T>(L, idx);
}

template <typename T>
T rvg_lua_check(lua_State *L, int idx, int ctxidx = lua_upvalueindex(1)) {
    if (!rvg_lua_is<T>(L, idx, ctxidx))
        rvg_lua_argerror<T>(L, idx, ctxidx);
    return rvg_lua_to<T>(L, idx);
}

template <typename T, typename U>
T rvg_lua_check_either(lua_State *L, int idx, int ctxidx = lua_upvalueindex(1)) {
     if (!rvg_lua_is<T>(L, idx, ctxidx)) {
        if (!rvg_lua_is<U>(L, idx, ctxidx))
            rvg_lua_argerror<T>(L, idx, ctxidx);
        return rvg_lua_to<U>(L, idx);
     }
    return rvg_lua_to<T>(L, idx);
}

template <>
int32_t rvg_lua_check<int32_t>(lua_State *L, int idx, int);

template <>
int64_t rvg_lua_check<int64_t>(lua_State *L, int idx, int);

template <>
float rvg_lua_check<float>(lua_State *L, int idx, int);

template <>
double rvg_lua_check<double>(lua_State *L, int idx, int);

extern template
int32_t rvg_lua_check<int32_t>(lua_State *L, int idx, int);

extern template
float rvg_lua_check<float>(lua_State *L, int idx, int);

extern template
int64_t rvg_lua_check<int64_t>(lua_State *L, int idx, int);

extern template
double rvg_lua_check<double>(lua_State *L, int idx, int);

template <typename T>
T rvg_lua_opt(lua_State *L, int idx, const T& def = T{},
    int ctxidx = lua_upvalueindex(1)) {
    if (rvg_lua_is<T>(L, idx, ctxidx)) {
        return rvg_lua_to<T>(L, idx);
	} else return def;
}

template <typename T>
void rvg_lua_setmetatable(lua_State *L, int objidx,
	int ctxidx = lua_upvalueindex(1)) {
    objidx = rvg_lua_abs_index(L, objidx);
    rvg_lua_gettypemetatable<T>(L, ctxidx);
    lua_setmetatable(L, objidx);
}

template <typename T, typename ...As, size_t ...Is>
int rvg_lua_new_helper(lua_State *L, std::index_sequence<Is...>) {
    int n = static_cast<int>(sizeof...(As));
    int t = lua_gettop(L);
    if (t < n) luaL_error(L, "not enough arguments (needed %d)", n);
    if (t > n) luaL_error(L, "too many arguments (needed %d)", n);
    T* p = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));
    new (p) T(rvg_lua_check<As>(L, Is+1)...);
    rvg_lua_setmetatable<T>(L, -1);
    return 1;
}

template <typename T, typename ...As>
int rvg_lua_new(lua_State *L) {
    return rvg_lua_new_helper<T, As...>(L,
        std::make_index_sequence<sizeof...(As)>{});
}

template <typename T>
int rvg_lua_push(lua_State *L, const T &value, int ctxidx = lua_upvalueindex(1)) {
    T* ptr = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));
    new (ptr) T{value};
    rvg_lua_setmetatable<T>(L, -1, ctxidx);
	return 1;
}

template <typename T>
int rvg_lua_push(lua_State *L, T &&value, int ctxidx = lua_upvalueindex(1)) {
    T* ptr = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));
    new (ptr) T{std::move(value)};
    rvg_lua_setmetatable<T>(L, -1, ctxidx);
	return 1;
}

template <>
int rvg_lua_push<float>(lua_State *L, const float &value, int);

template <>
int rvg_lua_push<int32_t>(lua_State *L, const int32_t &value, int);

template <>
int rvg_lua_push<double>(lua_State *L, const double &value, int);

template <>
int rvg_lua_push<int64_t>(lua_State *L, const int64_t &value, int);

template <>
int rvg_lua_push<const char *>(lua_State *L, const char * const & value, int);

template <>
int rvg_lua_push<float>(lua_State *L, float &&value, int);

template <>
int rvg_lua_push<int32_t>(lua_State *L, int32_t &&value, int);

template <>
int rvg_lua_push<double>(lua_State *L, double &&value, int);

template <>
int rvg_lua_push<int64_t>(lua_State *L, int64_t &&value, int);

template <>
int rvg_lua_push<const char *>(lua_State *L, const char * &&value, int);

extern template
int rvg_lua_push<float>(lua_State *L, const float &value, int);

extern template
int rvg_lua_push<int32_t>(lua_State *L, const int32_t &value, int);

extern template
int rvg_lua_push<double>(lua_State *L, const double &value, int);

extern template
int rvg_lua_push<int64_t>(lua_State *L, const int64_t &value, int);

extern template
int rvg_lua_push<const char *>(lua_State *L, const char * &&value, int);

extern template
int rvg_lua_push<float>(lua_State *L, float &&value, int);

extern template
int rvg_lua_push<int32_t>(lua_State *L, int32_t &&value, int);

extern template
int rvg_lua_push<double>(lua_State *L, double &&value, int);

extern template
int rvg_lua_push<int64_t>(lua_State *L, int64_t &&value, int);

extern template
int rvg_lua_push<const char *>(lua_State *L, const char * &&value, int);

template <typename T>
void rvg_lua_setmethods(lua_State *L, const luaL_Reg *methods,
    int nup, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx); // up1 .. upn
    rvg_lua_gettypemetatable<T>(L, ctxidx); // up1 .. upn meta
    lua_getfield(L, -1, "__index");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L); // up1 .. upn meta index
        lua_pushvalue(L, -1); // up1 .. upn meta index index
        lua_setfield(L, -3, "__index");
    } // up1 .. upn meta index
    lua_insert(L, -nup-2); // index up1 .. upn meta
    lua_pop(L, 1); // index up1 .. upn
    lua_pushvalue(L, ctxidx); // index up1 .. upn ctxtab
    rvg_lua_setfuncs(L, methods, nup+1); // index
    lua_pop(L, 1); //
}

template <typename T>
void rvg_lua_setmetamethods(lua_State *L, const luaL_Reg *methods,
    int nup, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx); // up1 .. upn
    rvg_lua_gettypemetatable<T>(L, ctxidx); // up1 .. upn meta
    lua_insert(L, -nup-1); // meta up1 .. upn
    lua_pushvalue(L, ctxidx); // meta up1 .. upn ctxtab
    rvg_lua_setfuncs(L, methods, nup+1); // meta
    lua_pop(L, 1); //
}

template <typename T>
void rvg_lua_createtype(lua_State *L, const char *name,
    int ctxidx, const luaL_Reg *meta = nullptr) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    // check if name is already taken
    lua_getfield(L, ctxidx, name);
    if (!lua_isnil(L, -1)) {
        luaL_error(L, "redefinition of %s type", name);
    }
    lua_pop(L, 1);
    // create new type
    luaL_Reg default_meta[] = {
        { "__gc", &rvg_lua_gc<T> },
        { "__tostring", &rvg_lua_tostring<T> },
        { nullptr, nullptr },
    };
    lua_pushstring(L, rvg_lua_raw_name<T>()); // T_raw_name
    lua_newtable(L); // T_raw_name T_meta
    lua_pushstring(L, name); // T_raw_name T_meta T_name
    lua_setfield(L, -2, "name"); // T_raw_name T_meta
    lua_pushvalue(L, ctxidx); // T_raw_name T_meta ctxtab
    rvg_lua_setfuncs(L, default_meta, 1); // T_raw_name T_meta
    // if there are metamethods to be overriden, do so
    if (meta) {
        lua_pushvalue(L, ctxidx); // T_raw_name T_meta ctxtab
        rvg_lua_setfuncs(L, meta, 1); // T_raw_name T_meta
    }
	lua_pushliteral(L, "access denied"); // T_raw_name T_meta "access denied"
	lua_setfield(L, -2, "__metatable"); // T_raw_name T_meta
    lua_rawset(L, ctxidx); //
}

template <typename T>
struct rvg_lua_named_value {
    const char *name;
    T value;
};

template <typename E>
int rvg_lua_enum_tostring(lua_State *L) {
    rvg_lua_gettypemetatable<E>(L); // meta
    lua_getfield(L, -1, "name"); // meta type_name
    lua_getfield(L, -2, "map"); // meta type_name map
    lua_pushvalue(L, 1); // meta type_name map value
    lua_gettable(L, -2); // meta type_name map value_name
    lua_pushfstring(L, "%s.%s",
        lua_tostring(L, -3), lua_tostring(L, -1));
    return 1;
}

template <typename E>
void rvg_lua_enum_createtype(lua_State *L, const char *name,
	rvg_lua_named_value<E> *pairs, int ctxidx = lua_upvalueindex(1)) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    rvg_lua_createtype<E>(L, name, ctxidx);
	rvg_lua_gettypemetatable<E>(L, ctxidx); // meta
	lua_newtable(L); // meta map
	while (pairs->name) {
        lua_pushstring(L, pairs->name); // meta map name
        rvg_lua_push<E>(L, pairs->value, ctxidx); // meta map name value
		lua_pushinteger(L, static_cast<lua_Integer>(pairs->value)); // meta map name value key
		lua_pushvalue(L, -2); // meta map name value key value
		lua_pushvalue(L, -1); // meta map name value key value value
		lua_pushvalue(L, -5); // meta map name value key value value name
		lua_settable(L, -7); // meta map name value key value
		lua_settable(L, -5); // meta map name value
		lua_settable(L, -3); // meta map
        pairs++;
    }
    lua_setfield(L, -2, "map"); // meta
    lua_pop(L, 1); //
    static luaL_Reg metaenum[] = {
        {"__tostring", &rvg_lua_enum_tostring<E>},
        {nullptr, nullptr}
    };
    rvg_lua_setmetamethods<E>(L, metaenum, 0, ctxidx);
}

template <typename E>
int rvg_lua_enum_push(lua_State *L, E value, int ctxidx = lua_upvalueindex(1)) {
    rvg_lua_gettypemetatable<E>(L, ctxidx); // meta
    lua_getfield(L, -1, "map"); // meta map
    lua_pushinteger(L, static_cast<lua_Integer>(value)); // meta map key
    lua_gettable(L, -2); // meta map value
    lua_replace(L, -3); // value map
    lua_pop(L, 1); // value
	return 1;
}

template <typename E>
void rvg_lua_enum_pushmap(lua_State *L,
    int ctxidx = lua_upvalueindex(1)) {
    rvg_lua_gettypemetatable<E>(L, ctxidx); // meta
    lua_getfield(L, -1, "map"); // meta map
    lua_replace(L, -2); // map
}


template <typename T, typename G>
class rvg_lua_table_iterator:
    public boost::iterator_facade<
        rvg_lua_table_iterator<T, G>, 
        T const, 
        boost::forward_traversal_tag, 
        T
    > {
    lua_State *m_L;  // lua context where table lives
    int m_tab;       // stack position of table in context
    int m_item;      // current item in table
    G m_get;
public:
    rvg_lua_table_iterator(lua_State *L, int tab, int item, G &&get):
        m_L(L),
        m_tab(rvg_lua_abs_index(L, tab)),
        m_item(item),
        m_get(std::forward<G>(get)) {
            ;
        }
private:
    friend class boost::iterator_core_access;

    void increment(void) {
        ++m_item;
    }

    bool equal(const rvg_lua_table_iterator<T, G> &other) const {
        return m_item == other.m_item;
    }

    T dereference(void) const {
        return m_get(m_L, m_tab, m_item);
    }
};

template <typename T, typename G>
inline std::pair<
    rvg_lua_table_iterator<T, G>,
    rvg_lua_table_iterator<T, G>
>
rvg_lua_make_table_range(lua_State *L, int tab, int beg, int end, G &&get) {
    return std::make_pair(
        rvg_lua_table_iterator<T,G>{L, tab, beg, get},
        rvg_lua_table_iterator<T,G>{L, tab, end, get}
    );
}

#endif
