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

namespace Data { class SignalConnectionRef; }

class SignalConnection : public std::enable_shared_from_this<SignalConnection> {
protected:
    std::weak_ptr<Signal> parentSignal;

    virtual void Call(std::vector<Data::Variant>) = 0;
    friend Signal;
public:
    inline bool Connected() { return !parentSignal.expired(); };
    void Disconnect();

    virtual ~SignalConnection();
};

class CSignalConnection : public SignalConnection {
    std::function<void(std::vector<Data::Variant>)> function;

    CSignalConnection(std::function<void(std::vector<Data::Variant>)>);

    friend Signal;
protected:
    void Call(std::vector<Data::Variant>) override;
};

class LuaSignalConnection : public SignalConnection {
    lua_State* thread;

    LuaSignalConnection(lua_State*);

    friend Signal;
protected:
    void Call(std::vector<Data::Variant>) override;
public:
    ~LuaSignalConnection();
};

class Signal {
    std::vector<std::shared_ptr<SignalConnection>> connections;

    friend SignalConnection;
public:
    Signal();
    virtual ~Signal();

    void DisconnectAll();
    void Fire(std::vector<Data::Variant> args);
    Data::SignalConnectionRef Connect(std::function<void(std::vector<Data::Variant>)> callback);
    Data::SignalConnectionRef Connect(lua_State*);
};

class SignalSource : public std::shared_ptr<Signal> {
public:
    SignalSource();
    virtual ~SignalSource();
};

namespace Data {
    class SignalRef : public Data::Base {
        std::weak_ptr<Signal> signal;

    public:
        SignalRef(std::weak_ptr<Signal>);
        ~SignalRef();

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        operator std::weak_ptr<Signal>();

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node node) const override;
        virtual void PushLuaValue(lua_State*) const override;
        static result<Data::Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
    };

    class SignalConnectionRef : public Data::Base {
        std::weak_ptr<SignalConnection> signalConnection;

    public:
        SignalConnectionRef(std::weak_ptr<SignalConnection>);
        ~SignalConnectionRef();

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        operator std::weak_ptr<SignalConnection>();

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node node) const override;
        virtual void PushLuaValue(lua_State*) const override;
        static result<Data::Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
    };
}

using Data::SignalRef;
using Data::SignalConnectionRef;