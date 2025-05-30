#pragma once

#include <Helpers/String.hpp>
#include <Unreal/FProperty.hpp>
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
                throw std::runtime_error("Tried to wrap null UObject");
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
                throw std::runtime_error(
                    RC::to_string(fmt::format(STR("Property {} not found on object"), PropertyName)));
            }
            return Property;
        }

        Detail::FProperty* GetPropertyByNameInChain(const TCHAR* PropertyName) const {
            auto Property = wrapped->GetPropertyByNameInChain(PropertyName);
            if (!Property) {
                throw std::runtime_error(
                    RC::to_string(fmt::format(STR("Property {} not found on object"), PropertyName)));
            }
            return Property;
        }

        UFunctionWrapper GetFunctionByName(const TCHAR* functionName) const;
        UFunctionWrapper GetFunctionByNameInChain(const TCHAR* functionName) const;

        template <RC::Unreal::UObjectPointerDerivativeOrAnyNonUObject T = void>
        T* GetMember(const TCHAR* PropertyName) const {
            auto Property = GetPropertyByName(PropertyName);
            return Property->ContainerPtrToValuePtr<T>(wrapped);
        }

        template <RC::Unreal::UObjectPointerDerivativeOrAnyNonUObject T = void>
        T* GetMemberInChain(const TCHAR* PropertyName) const {
            auto Property = GetPropertyByNameInChain(PropertyName);
            return Property->ContainerPtrToValuePtr<T>(wrapped);
        }

        template <typename T>
        void SetMember(const TCHAR* PropertyName, T value) const {
            auto Property = GetPropertyByName(PropertyName);
            auto ValuePtr = Property->ContainerPtrToValuePtr<T>(wrapped);
            *ValuePtr = value;
        }

        template <typename T>
        void SetMemberInChain(const TCHAR* PropertyName, T value) const {
            auto Property = GetPropertyByNameInChain(PropertyName);
            auto ValuePtr = Property->ContainerPtrToValuePtr<T>(wrapped);
            *ValuePtr = value;
        }

        template <typename R = void>
        R call(const UFunctionWrapper Function) const {
            Detail::FProperty* ReturnProperty = Function.value()->GetReturnProperty();
            if (!ReturnProperty) {
                throw std::runtime_error("Unable to get return property");
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

        template <typename R = void>
        R call(const TCHAR* FunctionName) const {
            auto Function = GetFunctionByNameInChain(FunctionName);
            return call<R>(Function);
        }

        void call(const TCHAR* FunctionName, void* Params) const {
            auto Function = GetFunctionByNameInChain(FunctionName);
            wrapped->ProcessEvent(Function.value(), Params);
        }

        void PrintAllPropertyNames();
        void PrintAllFunctionNames();
        void PrintAllChildren();
    };

}
