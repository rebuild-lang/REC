#pragma once
#include "Module.h"
#include "Scope.h"

namespace instance {

inline auto lookup(const Scope& scope, NameView name) -> const Entry& {
    auto it = name.begin();
    auto end = name.end();
    auto it2 = std::find(it, end, '.');
    auto range = scope[Name(it, it2)];
    if (!range.single()) throw "name not found";
    while (it2 != end) {
        it = it2;
        it2 = std::find(it, end, '.');
        auto& node = range.begin()->second;
        node.visit(
            [&](const Module& m) -> decltype(auto) {
                range = m.locals[Name{it, it2}];
            },
            [](const auto&) { throw "not a module!"; } //
        );
        if (!range.single()) throw "nested name not found";
    }
    return range.frontValue();
}

template<class T>
auto lookupA(const Scope& scope, NameView name) -> const T& {
    const auto& c = lookup(scope, name);
    if constexpr (std::is_same_v<T, Type>) {
        if (!c.holds<Module>()) throw "wrong type";
        const auto& m = c.get<Module>();
        auto tr = m.locals[Name{"type"}];
        if (!tr.single()) throw "wrong type";
        return tr.frontValue().get<T>();
    }
    else if (!c.holds<T>()) {
        throw "wrong type";
    }
    return c.get<T>();
}

} // namespace instance
