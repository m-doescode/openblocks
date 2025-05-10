#pragma once

#include "base.h"
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

class SignalConnection : public std::enable_shared_from_this<SignalConnection> {
protected:
    std::weak_ptr<Signal> parentSignal;

    virtual ~SignalConnection();

    virtual void Call(std::vector<Data::Variant>);
    friend Signal;
public:
    inline bool Connected() { return !parentSignal.expired(); };
    void Disconnect();
};

class CSignalConnection : protected SignalConnection {
    std::function<void(std::vector<Data::Variant>)> function;

    CSignalConnection(std::function<void(std::vector<Data::Variant>)>);

    friend Signal;
protected:
    void Call(std::vector<Data::Variant>) override;
};

class LuaSignalConnection : protected SignalConnection {
    lua_State* thread;

    LuaSignalConnection(lua_State*);
    ~LuaSignalConnection();

    friend Signal;
protected:
    void Call(std::vector<Data::Variant>) override;
};

class Signal {
    std::vector<std::shared_ptr<SignalConnection>> connections;

    friend SignalConnection;
public:
    Signal();
    virtual ~Signal();

    void DisconnectAll();
    void Fire(std::vector<Data::Variant> args);
    void Connect(std::function<void(std::vector<Data::Variant>)> callback);
    void Connect(lua_State*);
};

class SignalSource : public std::shared_ptr<Signal> {
public:
    SignalSource();
    virtual ~SignalSource();
};

namespace Data {
    class SignalRef : public Data::Base {
        std::weak_ptr<Signal> signal;
    };

    class SignalConnectionRef : public Data::Base {
        std::weak_ptr<Signal> signal;
    };
}