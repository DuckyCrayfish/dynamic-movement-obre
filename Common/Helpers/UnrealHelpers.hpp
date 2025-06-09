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
#include <LuaCustomMemberFunctions.hpp>
#include <UE4SSProgram.hpp>
#include <UnrealDef.hpp>
#include <expected>

#include "Shapes/FKey.hpp"
#include "Wrappers/Wrappers.hpp"

using namespace RC::Unreal;


namespace Helpers {
    Wrappers::PlayerControllerWrapper GetPlayerController() {
        static UObject* playerController = nullptr;

        if (playerController == nullptr) {
            playerController = UObjectGlobals::FindFirstOf(STR("PlayerController"));
        }
        if (playerController == nullptr) {
            throw std::runtime_error("PlayerController not found");
        }
        return Wrappers::PlayerControllerWrapper(playerController);
    }

    Wrappers::PlayerCharacterWrapper GetPlayerCharacter() {
        return GetPlayerController().GetPlayerCharacter();
    }

    Wrappers::CharacterMovementWrapper GetPlayerMovement() {
        return GetPlayerCharacter().GetCharacterMovement();
    }

    /** Returns true if `key` is pressed, otherwise false. */
    std::expected<bool, StringType> IsKeyPressed(FKey& key) {
        auto playerController = GetPlayerController();
        auto IsInputKeyDown = playerController.GetFunctionByNameInChain(STR("IsInputKeyDown"));

        struct Params {
            FKey Key;
            std::byte padding[8];
            bool ReturnValue;
        };
        static_assert(sizeof(Params) == 32, "Size mismatch");
        Params params{key};
        playerController.value()->ProcessEvent(IsInputKeyDown.value(), &params);
        return params.ReturnValue;
    }

    /** Returns true if `key` is pressed, otherwise false. */
    std::expected<bool, StringType> IsKeyPressed(StringType key) {
        FKey fKey = FKey(key);
        return IsKeyPressed(fKey);
    }

    /** Returns a pointer to the input mapping context if it exists. */
    Wrappers::UObjectWrapper GetInputMappingContext(const FName& Name) {
        static auto NameInputMappingContext = FName(STR("InputMappingContext"));
        auto imc = RC::Unreal::UObjectGlobals::FindObject(NameInputMappingContext, Name);
        if (!imc) {
            throw std::runtime_error("InputMappingContext object not found");
        }
        return Wrappers::UObjectWrapper(imc);
    }

    /** Returns a pointer to the input mapping context if it exists. */
    Wrappers::UObjectWrapper GetInputMappingContext(const StringType& NameStr) {
        auto name = FName(NameStr);
        return GetInputMappingContext(name);
    }

    /** Returns true if the specified input action is pressed, otherwise false. */
    bool IsInputActionKeyPressed(const StringType& InputMappingContextName, const StringType& InputActionName) {
        auto imc = GetInputMappingContext(InputMappingContextName);
        auto mappingsProperty = imc.GetMemberInChain<FScriptArray>(STR("Mappings"));

        auto mappings = StaticCast<FArrayProperty*>(imc.GetPropertyByNameInChain(STR("Mappings")));
        static auto SprintActionName = FName(InputActionName);
        const int32 ElementSize = mappings->GetInner()->GetElementSize();
        auto actionKeyMapping = static_cast<FStructProperty*>(mappings->GetInner());
        FProperty* keyProperty = actionKeyMapping->GetStruct()->GetPropertyByNameInChain(STR("Key"));
        if (!keyProperty) {
            throw std::runtime_error("Property 'Key' not found on UEnhancedActionKeyMapping");
        }
        FProperty* actionProperty = actionKeyMapping->GetStruct()->GetPropertyByNameInChain(STR("Action"));
        if (!actionProperty) {
            throw std::runtime_error("Property 'Action' not found on UEnhancedActionKeyMapping");
        }
        for (int32_t i = 0; i < mappingsProperty->Num(); i++) {
            const int32 offset = i * ElementSize;
            auto element = static_cast<uint8*>(mappingsProperty->GetData()) + offset;
            auto key = keyProperty->ContainerPtrToValuePtr<FKey>(element);
            auto action = *actionProperty->ContainerPtrToValuePtr<UObject*>(element);

            if (action->GetNamePrivate().Equals(SprintActionName) && IsKeyPressed(*key).value_or(false)) {
                return true;
            }
        }
        return false;
    }
}
