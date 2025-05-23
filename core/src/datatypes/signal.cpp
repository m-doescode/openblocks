#include "signal.h"
#include "datatypes/base.h"
#include "meta.h"
#include "lua.h"
#include <pugixml.hpp>
#include <memory>
#include <vector>

SignalSource::SignalSource() : std::shared_ptr<Signal>(std::make_shared<Signal>()) {}
SignalSource::~SignalSource() = default;

Signal::Signal() {}
Signal::~Signal() = default;

SignalConnection::SignalConnection(std::weak_ptr<Signal> parent) : parentSignal(parent) {}
SignalConnection::~SignalConnection() = default;

// Only used for its address
LuaSignalConnection::LuaSignalConnection(lua_State* L, std::weak_ptr<Signal> parent) : SignalConnection(parent) {
    state = L;

    // https://stackoverflow.com/a/31952046/16255372

    // Save function so it doesn't get GC'd
    function = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);
}

LuaSignalConnection::~LuaSignalConnection() {
    // Remove LuaSignalConnectionthread so that it can get properly GC'd
    luaL_unref(state, LUA_REGISTRYINDEX, function);
}

void LuaSignalConnection::Call(std::vector<Data::Variant> args) {
    lua_State* thread = lua_newthread(state);

    // Push function
    lua_rawgeti(thread, LUA_REGISTRYINDEX, function);
    luaL_unref(thread, LUA_REGISTRYINDEX, function);

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

CSignalConnection::CSignalConnection(std::function<void(std::vector<Data::Variant>)> func, std::weak_ptr<Signal> parent) : SignalConnection(parent) {
    this->function = func;
}

void CSignalConnection::Call(std::vector<Data::Variant> args) {
    function(args);
}

//

SignalConnectionHolder::SignalConnectionHolder() : heldConnection() {}
SignalConnectionHolder::SignalConnectionHolder(std::shared_ptr<SignalConnection> connection) : heldConnection(connection) {}
SignalConnectionHolder::SignalConnectionHolder(Data::SignalConnectionRef other) : heldConnection(other) {}

SignalConnectionHolder::~SignalConnectionHolder() {
    // printf("Prediscon!\n");
    // if (!heldConnection.expired()) printf("Disconnected!\n");
    if (!heldConnection.expired())
        heldConnection.lock()->Disconnect();
}

//

SignalConnectionRef Signal::Connect(std::function<void(std::vector<Data::Variant>)> callback) {
    auto conn = std::dynamic_pointer_cast<SignalConnection>(std::make_shared<CSignalConnection>(callback, weak_from_this()));
    connections.push_back(conn);
    return SignalConnectionRef(conn);
}

SignalConnectionRef Signal::Connect(lua_State* state) {
    auto conn = std::dynamic_pointer_cast<SignalConnection>(std::make_shared<LuaSignalConnection>(state, weak_from_this()));
    connections.push_back(conn);
    return SignalConnectionRef(conn);
}

SignalConnectionRef Signal::Once(std::function<void(std::vector<Data::Variant>)> callback) {
    auto conn = std::dynamic_pointer_cast<SignalConnection>(std::make_shared<CSignalConnection>(callback, weak_from_this()));
    onceConnections.push_back(conn);
    return SignalConnectionRef(conn);
}

SignalConnectionRef Signal::Once(lua_State* state) {
    auto conn = std::dynamic_pointer_cast<SignalConnection>(std::make_shared<LuaSignalConnection>(state, weak_from_this()));
    onceConnections.push_back(conn);
    return SignalConnectionRef(conn);
}

//

int Signal::Wait(lua_State* thread) {
    lua_pushthread(thread);
    int threadId = luaL_ref(thread, LUA_REGISTRYINDEX);
    waitingThreads.push_back(std::make_pair(threadId, thread));

    // Yield and return results
    return lua_yield(thread, 0);
}

void Signal::Fire(std::vector<Data::Variant> args) {
    for (std::shared_ptr<SignalConnection> connection : connections) {
        connection->Call(args);
    }

    // Call once connections
    auto prevOnceConns = std::move(onceConnections);
    onceConnections = std::vector<std::shared_ptr<SignalConnection>>();
    for (std::shared_ptr<SignalConnection> connection : prevOnceConns) {
        connection->Call(args);
    }

    // Call waiting threads
    auto prevThreads = std::move(waitingThreads);
    waitingThreads = std::vector<std::pair<int, lua_State*>>();
    for (auto& [threadId, thread] : prevThreads) {
        for (Data::Variant arg : args) {
            arg.PushLuaValue(thread);
        }

        int status = lua_resume(thread, args.size());
        if (status > LUA_YIELD) {
            Logger::error(lua_tostring(thread, -1));
            lua_pop(thread, 1); // Pop return value
        }

        // Remove thread from registry
        luaL_unref(thread, LUA_REGISTRYINDEX, threadId);
    }

}

void Signal::Fire() {
    return Fire(std::vector<Data::Variant> {});
}

void Signal::DisconnectAll() {
    for (std::shared_ptr<SignalConnection> connection : connections) {
        connection->parentSignal = {};
    }
    connections.clear();

    for (std::shared_ptr<SignalConnection> connection : onceConnections) {
        connection->parentSignal = {};
    }
    onceConnections.clear();

    for (auto& [threadId, thread] : waitingThreads) {
        luaL_unref(thread, -1, LUA_REGISTRYINDEX);
    }
    waitingThreads.clear();
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

    for(auto it = signal->onceConnections.begin(); it != signal->onceConnections.end();) {
        if (*it == shared_from_this())
            it = signal->onceConnections.erase(it);
        else
            it++;
    }

    parentSignal = {};
}

//

static int signal_Connect(lua_State*);
static int signal_Once(lua_State*);
static int signal_Wait(lua_State*);

static int signal_gc(lua_State*);
static int signal_index(lua_State*);
static int signal_tostring(lua_State*);
static const struct luaL_Reg signal_metatable [] = {
    {"__gc", signal_gc},
    {"__index", signal_index},
    {"__tostring", signal_tostring},
    {NULL, NULL} /* end of array */
};

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

    auto userdata = (std::weak_ptr<Signal>**)lua_newuserdata(L, sizeof(std::weak_ptr<Signal>));
    *userdata = new std::weak_ptr<Signal>(signal);

    // Create the instance's metatable
    luaL_newmetatable(L, "__mt_signal");
    luaL_register(L, NULL, signal_metatable);

    lua_setmetatable(L, n+1);
}

result<Data::Variant, LuaCastError> Data::SignalRef::FromLuaValue(lua_State* L, int idx) {
    auto userdata = (std::weak_ptr<Signal>**)luaL_checkudata(L, 1, "__mt_signal");
    lua_pop(L, 1);
    return Data::Variant(Data::SignalRef(**userdata));
}

static int signal_gc(lua_State* L) {
    // Destroy the contained shared_ptr
    auto userdata = (std::weak_ptr<Signal>**)luaL_checkudata(L, 1, "__mt_signal");
    delete *userdata;
    lua_pop(L, 1);

    return 0;
}

// __index(t,k)
static int signal_index(lua_State* L) {
    auto userdata = (std::weak_ptr<Signal>**)luaL_checkudata(L, 1, "__mt_signal");
    std::weak_ptr<Signal> signal = **userdata;
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    if (key == "Connect") {
        lua_pushcfunction(L, signal_Connect);
        return 1;
    } else if (key == "Once") {
        lua_pushcfunction(L, signal_Once);
        return 1;
    } else if (key == "Wait") {
        lua_pushcfunction(L, signal_Wait);
        return 1;
    }

    return luaL_error(L, "'%s' is not a valid member of %s", key.c_str(), "Signal");
}

static int signal_tostring(lua_State* L) {
    lua_pop(L, 1);
    lua_pushstring(L, "Signal");
    return 1;
}

static int signal_Connect(lua_State* L) {
    auto userdata = (std::weak_ptr<Signal>**)luaL_checkudata(L, 1, "__mt_signal");
    std::shared_ptr<Signal> signal = (**userdata).lock();
    luaL_checktype(L, 2, LUA_TFUNCTION);

    SignalConnectionRef ref = signal->Connect(L);
    ref.PushLuaValue(L);

    return 1;
}

static int signal_Once(lua_State* L) {
    auto userdata = (std::weak_ptr<Signal>**)luaL_checkudata(L, 1, "__mt_signal");
    std::shared_ptr<Signal> signal = (**userdata).lock();
    luaL_checktype(L, 2, LUA_TFUNCTION);

    SignalConnectionRef ref = signal->Once(L);
    ref.PushLuaValue(L);

    return 1;
}

static int signal_Wait(lua_State* L) {
    auto userdata = (std::weak_ptr<Signal>**)luaL_checkudata(L, 1, "__mt_signal");
    // TODO: Add expiry check here and everywhere else
    std::shared_ptr<Signal> signal = (**userdata).lock();

    return signal->Wait(L);
}

//

static int signalconnection_Disconnect(lua_State*);

static int signalconnection_gc(lua_State*);
static int signalconnection_index(lua_State*);
static int signalconnection_tostring(lua_State*);
static const struct luaL_Reg signalconnection_metatable [] = {
    {"__gc", signalconnection_gc},
    {"__index", signalconnection_index},
    {"__tostring", signalconnection_tostring},
    {NULL, NULL} /* end of array */
};

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

    auto userdata = (std::weak_ptr<SignalConnection>**)lua_newuserdata(L, sizeof(std::weak_ptr<SignalConnection>));
    *userdata = new std::weak_ptr<SignalConnection>(signalConnection);

    // Create the instance's metatable
    luaL_newmetatable(L, "__mt_signalconnection");
    luaL_register(L, NULL, signalconnection_metatable);

    lua_setmetatable(L, n+1);
}

result<Data::Variant, LuaCastError> Data::SignalConnectionRef::FromLuaValue(lua_State* L, int idx) {
    auto userdata = (std::weak_ptr<SignalConnection>**)luaL_checkudata(L, 1, "__mt_signalconnection");
    lua_pop(L, 1);
    return Data::Variant(Data::SignalConnectionRef(**userdata));
}

static int signalconnection_tostring(lua_State* L) {
    lua_pop(L, 1);
    lua_pushstring(L, "SignalConnection");
    return 1;
}

static int signalconnection_gc(lua_State* L) {
    // Destroy the contained shared_ptr
    auto userdata = (std::weak_ptr<SignalConnection>**)luaL_checkudata(L, 1, "__mt_signalconnection");
    delete *userdata;
    lua_pop(L, 1);

    return 0;
}

// __index(t,k)
static int signalconnection_index(lua_State* L) {
    auto userdata = (std::weak_ptr<SignalConnection>**)luaL_checkudata(L, 1, "__mt_signalconnection");
    std::weak_ptr<SignalConnection> signalConnection = **userdata;
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    if (key == "Disconnect") {
        lua_pushcfunction(L, signalconnection_Disconnect);
        return 1;
    }

    return luaL_error(L, "'%s' is not a valid member of %s", key.c_str(), "SignalConnection");
}

static int signalconnection_Disconnect(lua_State* L) {
    auto userdata = (std::weak_ptr<SignalConnection>**)luaL_checkudata(L, 1, "__mt_signalconnection");
    std::shared_ptr<SignalConnection> signal = (**userdata).lock();

    signal->Disconnect();

    return 0;
}