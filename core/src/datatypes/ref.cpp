#include "datatypes/ref.h"
#include "color3.h"
#include "meta.h" // IWYU pragma: keep
#include <memory>
#include "objects/base/instance.h"

Data::InstanceRef::InstanceRef() {};
Data::InstanceRef::InstanceRef(std::weak_ptr<Instance> instance) : ref(instance) {};
Data::InstanceRef::~InstanceRef() = default;

const Data::TypeInfo Data::InstanceRef::TYPE = {
    .name = "Instance",
    // .deserializer = &Data::InstanceRef::Deserialize,
};

const Data::TypeInfo& Data::InstanceRef::GetType() const { return Data::InstanceRef::TYPE; };

const Data::String Data::InstanceRef::ToString() const {
    return ref.expired() ? "" : ref.lock()->name;
}

Data::InstanceRef::operator std::weak_ptr<Instance>() {
    return ref;
}

// Serialization

void Data::InstanceRef::Serialize(pugi::xml_node node) const {
    // node.text().set(this->ToHex());
}

// Data::Variant Data::Color3::Deserialize(pugi::xml_node node) {
//     return Color3::FromHex(node.text().get());
// }
