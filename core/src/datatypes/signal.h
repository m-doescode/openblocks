#pragma once

#include "base.h"
#include "datatypes/annotation.h"
#include <functional>
#include <memory>
#include <vector>

// Ultimately, there's two routes:
// 1. Signals are tied to Instances and are basically just keys that refer to events in the instance
// 2. Signals are independent of Instances, but intermingled.
// I chose 2 because it gives Signals a higher class, and felt easier to implement
// This means that they can be used independently of Instance if need be in the future

class Instance;
class Signal;

class SignalConnectionRef;

class SignalConnection : public std::enable_shared_from_this<SignalConnection> {
protected:
    std::weak_ptr<Signal> parentSignal;

    SignalConnection(std::weak_ptr<Signal> parent);

    virtual void Call(std::vector<Variant>) = 0;
    friend Signal;
public:
    inline bool Connected() { return !parentSignal.expired(); };
    void Disconnect();

    virtual ~SignalConnection();
};

class CSignalConnection : public SignalConnection {
    std::function<void(std::vector<Variant>)> function;

    friend Signal;
protected:
    void Call(std::vector<Variant>) override;
public:
    CSignalConnection(std::function<void(std::vector<Variant>)>, std::weak_ptr<Signal> parent);
    virtual ~CSignalConnection() = default;
};

class LuaSignalConnection : public SignalConnection {
    lua_State* state;
    int function;

    friend Signal;
protected:
    void Call(std::vector<Variant>) override;
public:
    LuaSignalConnection(lua_State*, std::weak_ptr<Signal> parent);
    LuaSignalConnection (const LuaSignalConnection&) = delete;
    LuaSignalConnection& operator= (const LuaSignalConnection&) = delete;
    virtual ~LuaSignalConnection();
};

// Holds a signal connection such that when the holder is deleted (either via its parent object being deleted, or being overwritten),
// the connection is disconnected. Useful to prevent lingering connections that no longer contain valid objects
class SignalConnectionHolder {
    std::weak_ptr<SignalConnection> heldConnection;
public:
    SignalConnectionHolder();
    SignalConnectionHolder(std::shared_ptr<SignalConnection>);
    SignalConnectionHolder(SignalConnectionRef other);
    ~SignalConnectionHolder();

    // Prevent SignalConnectionHolder being accidentally copied, making it useless
    // https://stackoverflow.com/a/10473009/16255372
    SignalConnectionHolder(const SignalConnectionHolder&) = delete;
    SignalConnectionHolder& operator=(const SignalConnectionHolder&) = delete;
    SignalConnectionHolder(SignalConnectionHolder&&) = default;
    SignalConnectionHolder& operator=(SignalConnectionHolder&&) = default;

    inline bool Connected() { return !heldConnection.expired() && heldConnection.lock()->Connected(); }
    inline void Disconnect() { if (!heldConnection.expired()) heldConnection.lock()->Disconnect(); }
};

class Signal : public std::enable_shared_from_this<Signal> {
    std::vector<std::shared_ptr<SignalConnection>> connections;
    std::vector<std::shared_ptr<SignalConnection>> onceConnections;
    std::vector<std::pair<int, lua_State*>> waitingThreads;

    friend SignalConnection;
public:
    Signal();
    virtual ~Signal();
    Signal (const Signal&) = delete;
    Signal& operator= (const Signal&) = delete;

    void DisconnectAll();
    void Fire(std::vector<Variant> args);
    void Fire();
    SignalConnectionRef Connect(std::function<void(std::vector<Variant>)> callback);
    SignalConnectionRef Connect(lua_State*);
    SignalConnectionRef Once(std::function<void(std::vector<Variant>)> callback);
    SignalConnectionRef Once(lua_State*);
    int Wait(lua_State*);
};

class SignalSource : public std::shared_ptr<Signal> {
public:
    SignalSource();
    virtual ~SignalSource();
};

class SignalRef {
    std::weak_ptr<Signal> signal;

public:
    SignalRef(std::weak_ptr<Signal>);
    virtual ~SignalRef();

    static const TypeDesc TYPE;

    operator std::weak_ptr<Signal>();

    DEF_DATA_OP bool operator ==(SignalRef) const;

    virtual const std::string ToString() const;
    virtual void Serialize(pugi::xml_node node) const;
    virtual void PushLuaValue(lua_State*) const;
    static result<Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
};

class SignalConnectionRef {
    std::weak_ptr<SignalConnection> signalConnection;

public:
    SignalConnectionRef(std::weak_ptr<SignalConnection>);
    virtual ~SignalConnectionRef();

    static const TypeDesc TYPE;

    operator std::weak_ptr<SignalConnection>();

    DEF_DATA_OP bool operator ==(SignalConnectionRef) const;

    virtual const std::string ToString() const;
    virtual void Serialize(pugi::xml_node node) const;
    virtual void PushLuaValue(lua_State*) const;
    static result<Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
};