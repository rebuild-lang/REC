#pragma once
#include "Parameter.h"

#include "Scope.h"
#include "ScopeLookup.h"

#include <cassert>
#include <functional>

namespace instance {

namespace details {

using TypeBuilder = std::function<auto(const Scope&)->parser::TypeView>;

struct ParameterBuilder {
    using This = ParameterBuilder;
    Parameter arg{};
    TypeBuilder typeBuilder{};

    template<size_t N>
    explicit ParameterBuilder(const char (&name)[N]) {
        arg.typed.name = Name{name};
    }

    template<class Builder>
    auto type(Builder&& b) && -> ParameterBuilder {
        typeBuilder = [b2 = std::move(b)](const Scope& scope) mutable { return std::move(b2).build(scope); };
        return std::move(*this);
    }

    auto left() && -> ParameterBuilder {
        arg.side = ParameterSide::left;
        return std::move(*this);
    }
    auto right() && -> ParameterBuilder {
        arg.side = ParameterSide::right;
        return std::move(*this);
    }
    auto result() && -> ParameterBuilder {
        arg.side = ParameterSide::result;
        return std::move(*this);
    }
    auto optional() && -> ParameterBuilder {
        arg.flags |= ParameterFlag::optional;
        return std::move(*this);
    }

    auto build(const Scope& scope, LocalScope& funScope) && -> ParameterView {
        if (typeBuilder) {
            arg.typed.type = typeBuilder(scope);
        }
        auto entry = funScope.emplace(std::move(arg));
        return &entry->get<Parameter>();
    }
};

} // namespace details

template<size_t N>
auto param(const char (&name)[N]) {
    return details::ParameterBuilder{name};
}

} // namespace instance
