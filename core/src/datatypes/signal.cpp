#include "signal.h"
#include "meta.h"
#include "lua.h"
#include <luajit-2.1/lua.h>
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
    connections.push_back(std::dynamic_pointer_cast<SignalConnection>(std::make_shared<CSignalConnection>(callback)));
}

void Signal::Connect(lua_State* state) {
    connections.push_back(std::dynamic_pointer_cast<SignalConnection>(std::make_shared<LuaSignalConnection>(state)));
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