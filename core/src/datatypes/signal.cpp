#include "signal.h"
#include "datatypes/base.h"
#include "meta.h"
#include "lua.h"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <pugixml.hpp>
#include <memory>

SignalSource::SignalSource() {}
SignalSource::~SignalSource() = default;

Signal::Signal() {}
Signal::~Signal() = default;

SignalConnection::~SignalConnection() = default;

// Only used for its address
int __savedThreads;
LuaSignalConnection::LuaSignalConnection(lua_State* L) {
    // Create thread from function at top of stack
    thread = lua_newthread(L);
    lua_xmove(L, thread, 1);
    
    // Save thread so it doesn't get GC'd
    lua_pushlightuserdata(thread, &__savedThreads);
    lua_gettable(thread, LUA_REGISTRYINDEX);

    lua_pushthread(thread); // key
    lua_pushboolean(thread, true); // value
    lua_rawset(thread, -3); // set

    lua_pop(thread, 1); // Pop __savedThreads
}

LuaSignalConnection::~LuaSignalConnection() {
    // Remove thread so that it can get properly GC'd
    lua_pushlightuserdata(thread, &__savedThreads);
    lua_gettable(thread, LUA_REGISTRYINDEX);

    lua_pushthread(thread); // key
    lua_pushnil(thread); // value
    lua_rawset(thread, -3); // set

    lua_pop(thread, 1); // Pop __savedThreads
}

void LuaSignalConnection::Call(std::vector<Data::Variant> args) {
    for (Data::Variant arg : args) {
        arg.PushLuaValue(thread);
    }

    int status = lua_resume(thread, args.size());
    if (status > LUA_YIELD) {
        Logger::error(lua_tostring(thread, -1));
        lua_pop(thread, 1); // Pop return value
    }
}

//

CSignalConnection::CSignalConnection(std::function<void(std::vector<Data::Variant>)> func) {
    this->function = func;
}

void CSignalConnection::Call(std::vector<Data::Variant> args) {
    function(args);
}

//

void Signal::Connect(std::function<void(std::vector<Data::Variant>)> callback) {
    connections.push_back(std::dynamic_pointer_cast<SignalConnection>(std::make_shared<CSignalConnection>(CSignalConnection(callback))));
}

void Signal::Connect(lua_State* state) {
    connections.push_back(std::dynamic_pointer_cast<SignalConnection>(std::make_shared<LuaSignalConnection>(LuaSignalConnection(state))));
}

void Signal::Fire(std::vector<Data::Variant> args) {
    for (std::shared_ptr<SignalConnection> connection : connections) {
        connection->Call(args);
    }
}

void Signal::DisconnectAll() {
    for (std::shared_ptr<SignalConnection> connection : connections) {
        connection->parentSignal = {};
    }
    connections.clear();
}

void SignalConnection::Disconnect() {
    if (!Connected()) return;
    auto signal = parentSignal.lock();
    for(auto it = signal->connections.begin(); it != signal->connections.end();) {
        if (*it == shared_from_this())
            it = signal->connections.erase(it);
        else
            it++;
    }
    parentSignal = {};
}

//

static int signal_gc(lua_State*);
static int signal_index(lua_State*);
static int signal_tostring(lua_State*);
static const struct luaL_Reg signal_metatable [] = {
    {"__gc", signal_gc},
    {"__index", signal_index},
    {"__tostring", signal_tostring},
    {NULL, NULL} /* end of array */
};


static int signal_gc(lua_State* L) {
    // Destroy the contained shared_ptr
    auto userdata = (std::weak_ptr<Signal>*)luaL_checkudata(L, 1, "__mt_signal");
    delete userdata;
    lua_pop(L, 1);

    return 0;
}

// __index(t,k)
static int signal_index(lua_State* L) {
    auto userdata = (std::weak_ptr<Signal>*)luaL_checkudata(L, 1, "__mt_signal");
    std::weak_ptr<Signal> signal = *userdata;
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    return luaL_error(L, "'%s' is not a valid member of %s", key.c_str(), "Signal");
}

Data::SignalRef::SignalRef(std::weak_ptr<Signal> ref) : signal(ref) {}
Data::SignalRef::~SignalRef() = default;

const Data::TypeInfo Data::SignalRef::TYPE = {
    .name = "Signal",
    .fromLuaValue = &Data::SignalRef::FromLuaValue,
};

const Data::TypeInfo& Data::SignalRef::GetType() const { return Data::SignalRef::TYPE; };

const Data::String Data::SignalRef::ToString() const {
    return Data::String("Signal");
}

Data::SignalRef::operator std::weak_ptr<Signal>() {
    return signal;
}

void Data::SignalRef::Serialize(pugi::xml_node node) const {
    // Not serializable
}

void Data::SignalRef::PushLuaValue(lua_State* L) const {
    int n = lua_gettop(L);

    auto userdata = (std::weak_ptr<Signal>*)lua_newuserdata(L, sizeof(void*));
    new(userdata) std::weak_ptr(signal);

    // Create the instance's metatable
    luaL_newmetatable(L, "__mt_signal");
    luaL_register(L, NULL, signal_metatable);

    lua_setmetatable(L, n+1);
}

result<Data::Variant, LuaCastError> Data::SignalRef::FromLuaValue(lua_State* L, int idx) {
    auto userdata = (std::weak_ptr<Signal>*)luaL_checkudata(L, 1, "__mt_signal");
    lua_pop(L, 1);
    return Data::Variant(Data::SignalRef(*userdata));
}

static int signal_tostring(lua_State* L) {
    lua_pop(L, 1);
    lua_pushstring(L, "Signal");
    return 1;
}

//

static int signalconnection_gc(lua_State*);
static int signalconnection_index(lua_State*);
static int signalconnection_tostring(lua_State*);
static const struct luaL_Reg signalconnection_metatable [] = {
    {"__gc", signalconnection_gc},
    {"__index", signalconnection_index},
    {"__tostring", signalconnection_tostring},
    {NULL, NULL} /* end of array */
};


static int signalconnection_gc(lua_State* L) {
    // Destroy the contained shared_ptr
    auto userdata = (std::weak_ptr<SignalConnection>*)luaL_checkudata(L, 1, "__mt_signalconnection");
    delete userdata;
    lua_pop(L, 1);

    return 0;
}

// __index(t,k)
static int signalconnection_index(lua_State* L) {
    auto userdata = (std::weak_ptr<SignalConnection>*)luaL_checkudata(L, 1, "__mt_signalconnection");
    std::weak_ptr<SignalConnection> signalConnection = *userdata;
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    return luaL_error(L, "'%s' is not a valid member of %s", key.c_str(), "SignalConnection");
}

Data::SignalConnectionRef::SignalConnectionRef(std::weak_ptr<SignalConnection> ref) : signalConnection(ref) {}
Data::SignalConnectionRef::~SignalConnectionRef() = default;

const Data::TypeInfo Data::SignalConnectionRef::TYPE = {
    .name = "Signal",
    .fromLuaValue = &Data::SignalConnectionRef::FromLuaValue,
};

const Data::TypeInfo& Data::SignalConnectionRef::GetType() const { return Data::SignalConnectionRef::TYPE; };

const Data::String Data::SignalConnectionRef::ToString() const {
    return Data::String("Connection");
}

Data::SignalConnectionRef::operator std::weak_ptr<SignalConnection>() {
    return signalConnection;
}

void Data::SignalConnectionRef::Serialize(pugi::xml_node node) const {
    // Not serializable
}

void Data::SignalConnectionRef::PushLuaValue(lua_State* L) const {
    int n = lua_gettop(L);

    auto userdata = (std::weak_ptr<SignalConnection>*)lua_newuserdata(L, sizeof(void*));
    new(userdata) std::weak_ptr(signalConnection);

    // Create the instance's metatable
    luaL_newmetatable(L, "__mt_signalconnection");
    luaL_register(L, NULL, signalconnection_metatable);

    lua_setmetatable(L, n+1);
}

result<Data::Variant, LuaCastError> Data::SignalConnectionRef::FromLuaValue(lua_State* L, int idx) {
    auto userdata = (std::weak_ptr<SignalConnection>*)luaL_checkudata(L, 1, "__mt_signalconnection");
    lua_pop(L, 1);
    return Data::Variant(Data::SignalConnectionRef(*userdata));
}

static int signalconnection_tostring(lua_State* L) {
    lua_pop(L, 1);
    lua_pushstring(L, "SignalConnection");
    return 1;
}