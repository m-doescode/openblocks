#pragma once

template <typename T> struct enum_type_of_t { static_assert(false, "Unknown enum type"); };

#define DEF_ENUM_META(_Enum) \
template <> struct enum_type_of_t<_Enum> { const Enum& value = EnumType::_Enum; };
