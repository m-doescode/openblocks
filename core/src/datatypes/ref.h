#pragma once

#include "base.h"
#include <memory>

class Instance;

namespace Data {
    class InstanceRef : Base {
        std::weak_ptr<Instance> ref;
    public:
        InstanceRef();
        InstanceRef(std::weak_ptr<Instance>);
        ~InstanceRef();

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        operator std::weak_ptr<Instance>();

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node node) const override;
        virtual void PushLuaValue(lua_State*) const override;
        // static Data::Variant Deserialize(pugi::xml_node node);
    };
}
