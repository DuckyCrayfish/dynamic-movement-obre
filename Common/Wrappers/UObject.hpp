/*
 * Copyright (C) 2025 Nick Iacullo
 *
 * This file is part of Dynamic Movement.
 *
 * Dynamic Movement is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Dynamic Movement is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Dynamic Movement.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <Helpers/String.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UClass.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UObjectGlobals.hpp>

#include "UFunction.hpp"


namespace Wrappers::Detail {
    using UObject = RC::Unreal::UObject;
    using FProperty = RC::Unreal::FProperty;
}

namespace Wrappers {

    class UObjectWrapper {
      protected:
        Detail::UObject* wrapped;

      public:
        UObjectWrapper(Detail::UObject* obj) : wrapped(obj) {
            if (obj == nullptr) {
                throw std::runtime_error("Tried to wrap null UObject pointer");
            }
        }

        Detail::UObject* operator->() {
            return wrapped;
        }
        const Detail::UObject* operator->() const {
            return wrapped;
        }

        Detail::UObject* value() const {
            return wrapped;
        }

        Detail::FProperty* GetPropertyByName(const TCHAR* PropertyName) const {
            auto Property = wrapped->GetPropertyByName(PropertyName);
            if (!Property) {
                throw std::runtime_error(RC::to_string(fmt::format(STR("Property {} not found on object of class {}"),
                                                                   PropertyName,
                                                                   wrapped->GetClassPrivate()->GetName())));
            }
            return Property;
        }

        /** Gets a property by name, throwing an error if it does not exist. */
        Detail::FProperty* GetPropertyByNameInChain(const TCHAR* PropertyName) const {
            auto Property = wrapped->GetPropertyByNameInChain(PropertyName);
            if (!Property) {
                throw std::runtime_error(RC::to_string(fmt::format(STR("Property {} not found on object of class {}"),
                                                                   PropertyName,
                                                                   wrapped->GetClassPrivate()->GetName())));
            }
            return Property;
        }

        UFunctionWrapper GetFunctionByName(const TCHAR* functionName) const;
        UFunctionWrapper GetFunctionByNameInChain(const TCHAR* functionName) const;

        /** Shortcut for finding a property by name and getting its contained value pointer. */
        template <RC::Unreal::UObjectPointerDerivativeOrAnyNonUObject T>
        T* GetMember(const TCHAR* PropertyName) const {
            auto Property = GetPropertyByName(PropertyName);
            return Property->ContainerPtrToValuePtr<T>(wrapped);
        }

        /** Shortcut for finding a property by name in chain and getting its contained value pointer. */
        template <RC::Unreal::UObjectPointerDerivativeOrAnyNonUObject T>
        T* GetMemberInChain(const TCHAR* PropertyName) const {
            auto Property = GetPropertyByNameInChain(PropertyName);
            return Property->ContainerPtrToValuePtr<T>(wrapped);
        }

        /** Shorthand for finding a property by name and setting it to `value`. */
        template <typename T>
        void SetMemberInChain(const TCHAR* PropertyName, T value) const {
            auto Property = GetPropertyByNameInChain(PropertyName);
            auto ValuePtr = Property->ContainerPtrToValuePtr<T>(wrapped);
            *ValuePtr = value;
        }

        /** Shorthand for finding a property by name in chain and setting it to `value`. */
        template <typename T>
        void SetMember(const TCHAR* PropertyName, T value) const {
            auto Property = GetPropertyByName(PropertyName);
            auto ValuePtr = Property->ContainerPtrToValuePtr<T>(wrapped);
            *ValuePtr = value;
        }

        template <typename R = void>
        R call(const UFunctionWrapper Function) const {
            if constexpr (std::is_void_v<R>) {
                void* Params = _malloca(Function.value()->GetParmsSize());
                wrapped->ProcessEvent(Function.value(), Params);
            } else {
                Detail::FProperty* ReturnProperty = Function.value()->GetReturnProperty();
                if (!ReturnProperty) {
                    throw std::runtime_error("Unable to fetch return property");
                }
                void* Params = _malloca(Function.value()->GetParmsSize());
                ReturnProperty->InitializeValue_InContainer(Params);

                wrapped->ProcessEvent(Function.value(), Params);
                void* ReturnContainer = ReturnProperty->ContainerPtrToValuePtr<void>(Params);
                if (!ReturnContainer) {
                    throw std::runtime_error("Unable to get return property container");
                }
                R* value = ReturnProperty->ContainerPtrToValuePtr<R>(ReturnContainer);
                if (value == nullptr) {
                    throw std::runtime_error("Unable to get return value");
                }
                return *value;
            }
        }

        template <typename R = void>
        R call(const TCHAR* FunctionName) const {
            auto Function = GetFunctionByNameInChain(FunctionName);
            return call<R>(Function);
        }

        void call(const TCHAR* FunctionName, void* Params) const {
            auto Function = GetFunctionByNameInChain(FunctionName);
            wrapped->ProcessEvent(Function.value(), Params);
        }

        /** Prints all contained property names to the output log. */
        void PrintAllPropertyNames();
        /** Prints all contained function names to the output log. */
        void PrintAllFunctionNames();
        /** Prints all contained function and property names to the output log. */
        void PrintAllChildren();
    };

}
