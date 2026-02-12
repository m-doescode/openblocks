#include <map>
#include "meta.h"

#include "datatypes/enummeta.h"
#include "enum/part.h"
#include "enum/surface.h"

template <typename... E>
std::map<std::string, const Enum*> make_enum_map() {
    std::map<std::string, const Enum*> map;
    ((map[enum_type_of_t<E>().value.InternalType()->name] = &enum_type_of_t<E>().value), ...);
    return map;
}

std::map<std::string, const Enum*> ENUM_MAP = make_enum_map<PartType, SurfaceType, NormalId>();