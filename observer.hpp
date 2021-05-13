#pragma once

#include <vector>
#include <qregexp.h>

#define cast_int(a) static_cast<int>(a)
#define cast_enum(e, a) static_cast<e>(a)

#define DEFINE_ENUM_FLAGS(ENUMTYPE) \
inline ENUMTYPE operator |(ENUMTYPE a, ENUMTYPE b) {                           \
    return cast_enum(ENUMTYPE, cast_int(a) | cast_int(b));                     \
}                                                                              \
inline ENUMTYPE operator &(ENUMTYPE a, ENUMTYPE b) {                           \
    return cast_enum(ENUMTYPE, cast_int(a) & cast_int(b));                     \
}                                                                              \
inline ENUMTYPE operator ^(ENUMTYPE a, ENUMTYPE b) {                           \
    return cast_enum(ENUMTYPE, cast_int(a) ^ cast_int(b));                     \
}                                                                              \
inline ENUMTYPE operator ~(ENUMTYPE a) {                                       \
    return cast_enum(ENUMTYPE, ~cast_int(a));                                  \
}                                                                              \
inline ENUMTYPE &operator |=(ENUMTYPE &a, ENUMTYPE b) { a = a | b; return a; } \
inline ENUMTYPE &operator &=(ENUMTYPE &a, ENUMTYPE b) { a = a & b; return a; } \
inline ENUMTYPE &operator ^=(ENUMTYPE &a, ENUMTYPE b) { a = a ^ b; return a; }


enum UserType {
    None  = 0,
    Mod   = 1 << 0,
    Sub   = 1 << 1,
    Vip   = 1 << 2,
    Stuff = 1 << 3,
};
DEFINE_ENUM_FLAGS(UserType)

QT_FORWARD_DECLARE_CLASS(ActionHandler)

enum ActionType {
    Toggle = 1 << 0,
    Hide   = 1 << 1,
    Show   = 1 << 2,
};

struct action_descriptor {
    ActionType type;
    std::vector<std::string> sceneitems;
    std::vector<std::string> users;
    std::string expression;
    bool is_active = true;
    int timeout = 0;
    ActionHandler *handler = nullptr;

    ~action_descriptor();
};

struct observer_settings {
    std::string channel;
    std::vector<action_descriptor> actions = std::vector<action_descriptor>();
};