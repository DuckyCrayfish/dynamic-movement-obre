#pragma once

#include <Helpers/String.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/Property/FObjectProperty.hpp>
#include <Unreal/UObject.hpp>

#include "CharacterMovement.hpp"
#include "UObject.hpp"


namespace Wrappers::Detail {
    using UObject = RC::Unreal::UObject;
    using FProperty = RC::Unreal::FProperty;
    using FObjectProperty = RC::Unreal::FObjectProperty;
}

namespace Wrappers {

    class PlayerCharacterWrapper : public UObjectWrapper {
      public:
        PlayerCharacterWrapper(Detail::UObject* obj) : UObjectWrapper(obj) {}

        CharacterMovementWrapper GetCharacterMovement();

        UObjectWrapper GetHorse() const {
            auto GetHorse = GetFunctionByNameInChain(STR("GetHorse")).value();
            Detail::FObjectProperty* ReturnProperty = CastField<Detail::FObjectProperty>(GetHorse->GetReturnProperty());
            void* Params = _malloca(GetHorse->GetParmsSize());
            ReturnProperty->InitializeValue_InContainer(Params);


            wrapped->ProcessEvent(GetHorse, Params);
            return UObjectWrapper(ReturnProperty->GetObjectPropertyValue(Params));
        }
    };

}
