#pragma once

#include <string>


#define DEF_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE) class CLASS_NAME : public Data::Base { \
    const WRAPPED_TYPE value; \
public: \
    CLASS_NAME(WRAPPED_TYPE); \
    ~CLASS_NAME(); \
    operator WRAPPED_TYPE(); \
    virtual const TypeInfo& GetType() const override; \
    static const TypeInfo TYPE; \
    \
    virtual const Data::String ToString() const override; \
};

namespace Data {
    struct TypeInfo {
        std::string name;
        TypeInfo(const TypeInfo&) = delete;
    };

    class String;
    class Base {
    public:
        virtual ~Base();
        virtual const TypeInfo& GetType() const = 0;
        virtual const Data::String ToString() const = 0;
    };

    class Null : Base {
    public:
        Null();
        ~Null();
        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        virtual const Data::String ToString() const override;
    };

    DEF_WRAPPER_CLASS(Bool, bool)
    DEF_WRAPPER_CLASS(Int, int)
    DEF_WRAPPER_CLASS(Float, float)
    DEF_WRAPPER_CLASS(String, std::string)
};


#undef DEF_WRAPPER_CLASS