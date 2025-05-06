#pragma once

#include <string>
#include <functional>
#include <optional>
#include "error/result.h"
#include "error/data.h"

extern "C" { typedef struct lua_State lua_State; }

namespace pugi { class xml_node; };

#define DEF_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE) class CLASS_NAME : public Data::Base { \
    WRAPPED_TYPE value; \
public: \
    CLASS_NAME(WRAPPED_TYPE); \
    ~CLASS_NAME(); \
    operator const WRAPPED_TYPE() const; \
    virtual const TypeInfo& GetType() const override; \
    static const TypeInfo TYPE; \
    \
    virtual const Data::String ToString() const override; \
    virtual void Serialize(pugi::xml_node node) const override; \
    virtual void PushLuaValue(lua_State*) const override; \
    \
    static Data::Variant Deserialize(pugi::xml_node node); \
    static std::optional<Data::Variant> FromString(std::string); \
    static result<Data::Variant, LuaCastError> FromLuaValue(lua_State*, int idx); \
};

namespace Data {
    class Variant;
    typedef std::function<Data::Variant(pugi::xml_node)> Deserializer;
    typedef std::function<std::optional<Data::Variant>(std::string)> FromString;
    typedef std::function<result<Data::Variant, LuaCastError>(lua_State*, int idx)> FromLuaValue;

    struct TypeInfo {
        std::string name;
        Deserializer deserializer;
        FromString fromString;
        FromLuaValue fromLuaValue;
    };

    class String;
    class Base {
    public:
        virtual ~Base();
        virtual const TypeInfo& GetType() const = 0;
        virtual const Data::String ToString() const = 0;
        virtual void Serialize(pugi::xml_node node) const = 0;
        virtual void PushLuaValue(lua_State*) const = 0;
    };

    class Null : Base {
    public:
        Null();
        ~Null();
        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node node) const override;
        virtual void PushLuaValue(lua_State*) const override;

        static Data::Variant Deserialize(pugi::xml_node node);
        static result<Data::Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
    };

    DEF_WRAPPER_CLASS(Bool, bool)
    DEF_WRAPPER_CLASS(Int, int)
    DEF_WRAPPER_CLASS(Float, float)
    DEF_WRAPPER_CLASS(String, std::string)
};


#undef DEF_WRAPPER_CLASS