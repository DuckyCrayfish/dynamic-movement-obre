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

        /// Returns the character movement component associated with this player character.
        CharacterMovementWrapper GetCharacterMovement();

        /// Returns the player's horse actor.
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
