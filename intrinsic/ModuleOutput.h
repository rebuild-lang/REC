#pragma once
#include "Function.h"
#include "Module.h"
#include "Type.h"

#include "strings/Output.h"

#include <iostream>

namespace intrinsic {

// example visitor implemenetation
struct ModuleOutput {
    using This = ModuleOutput;

    template<class T>
    void type() {
        constexpr auto info = TypeOf<T>::info();
        std::cout << indent << "type " << info.name << '\n';
        if constexpr (info.flags.any(TypeFlag::Instance)) {
            // TODO
        }
        else {
            indent += "  ";
            TypeOf<T>::module(*this);
            indent.resize(indent.size() - 2);
        }
    }

    template<class T>
    void module() {
        auto info = T::info();
        std::cout << indent << "module " << info.name << '\n';
        indent += "  ";
        T::module(*this);
        indent.resize(indent.size() - 2);
    }

    using FunctionInfoFunc = FunctionInfo (*)();

    template<FunctionInfoFunc Info, class... Args>
    void function(void (*func)(Args...)) {
        auto info = Info();
        std::cout << indent << "function " << info.name << '\n';
        indent += "  ";
        (argument<Args>(), ...);
        indent.resize(indent.size() - 2);
    }

private:
    std::string indent{};

    template<class T>
    void argument() {
        std::cout << indent << "arg " << Argument<T>::info().name //
                  << " : " << Argument<T>::typeInfo().name << '\n';
    }
};

} // namespace intrinsic