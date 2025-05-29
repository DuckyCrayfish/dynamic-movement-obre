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
#include <Mod/CppUserModBase.hpp>
#include <UE4SSProgram.hpp>
#include <UnrealDef.hpp>
#include <expected>

#include "../Shapes/FKey.hpp"

using namespace RC::Unreal;
using FunctionContext = Unreal::UnrealScriptFunctionCallableContext;
using UnrealScriptFunctionCallable = Unreal::UnrealScriptFunctionCallable;


namespace ModUtils {

    class ModBase : public RC::CppUserModBase {
      public:
        ~ModBase() {
            for (auto it = hookRegistrations.rbegin(); it != hookRegistrations.rend(); ++it) {
                const auto& [hookName, handle, unregister] = *it;
                if (unregister) {
                    RC::Unreal::UObjectGlobals::UnregisterHook(hookName, {handle, unregister});
                }
            }
        }

      protected:
        std::vector<std::tuple<StringType, int, int>> hookRegistrations;

        std::pair<int, int> RegisterModHook(const StringType& functionFullNameNoType,
                                            std::function<void()> preCallback) {
            return RegisterModHook(
                functionFullNameNoType,
                [preCallback](FunctionContext&, void*) { preCallback(); },
                [](FunctionContext&, void*) {},
                nullptr);
        }

        std::pair<int, int> RegisterModHook(const StringType& functionFullNameNoType,
                                            UnrealScriptFunctionCallable preCallback) {
            return RegisterModHook(functionFullNameNoType, preCallback, [](FunctionContext&, void*) {}, nullptr);
        }

        std::pair<int, int> RegisterModHook(const StringType& functionFullNameNoType, std::function<void()> preCallback,
                                            std::function<void()> postCallback) {
            return RegisterModHook(
                functionFullNameNoType,
                [preCallback](FunctionContext&, void*) { preCallback(); },
                [postCallback](FunctionContext&, void*) { postCallback(); },
                nullptr);
        }

        std::pair<int, int> RegisterModHook(const StringType& functionFullNameNoType,
                                            UnrealScriptFunctionCallable preCallback,
                                            UnrealScriptFunctionCallable postCallback) {
            return RegisterModHook(functionFullNameNoType, preCallback, postCallback, nullptr);
        }

        std::pair<int, int> RegisterModHook(const StringType& functionFullNameNoType,
                                            UnrealScriptFunctionCallable preCallback,
                                            UnrealScriptFunctionCallable postCallback, void* customData) {
            auto [preId, postId] =
                RC::Unreal::UObjectGlobals::RegisterHook(functionFullNameNoType, preCallback, postCallback, customData);
            hookRegistrations.emplace_back(functionFullNameNoType, preId, postId);
            return {preId, postId};
        }
    };

    /** Variant of `CastField` that throws an error if the cast fails. */
    template <RC::Unreal::FFieldDerivative T>
    inline auto CastFieldUnsafe(FField* field) -> T* {
        if (field == nullptr) {
            throw std::runtime_error("Field is null");
        }
        if (!field->IsA<T>()) {
            throw std::runtime_error("Field is not of the expected type");
        }
        return static_cast<T*>(field);
    }


    /** Fetches a pointer to the `UFunction` with the name `functionName` on the UObject `obj`. */
    std::expected<UFunction*, StringType> getFunctionByNameInChain(UObject* obj, const TCHAR* functionName) {
        UFunction* function = obj->GetFunctionByNameInChain(functionName);
        if (function == nullptr) {
            return std::unexpected(STR("Function ") + std::wstring(functionName) + STR(" not found."));
        }
        return function;
    }

    std::expected<bool, StringType> callPropositional(UObject* obj, const TCHAR* functionName) {
        auto Function = getFunctionByNameInChain(obj, functionName);
        if (!Function) {
            return std::unexpected(Function.error());
        }
        FProperty* ReturnProperty = Function.value()->GetReturnProperty();
        void* Params = _malloca(Function.value()->GetParmsSize());
        ReturnProperty->InitializeValue_InContainer(Params);

        obj->ProcessEvent(Function.value(), Params);
        FBoolProperty* ReturnBoolProperty = ReturnProperty->ContainerPtrToValuePtr<FBoolProperty>(Params);
        bool* value = ReturnProperty->ContainerPtrToValuePtr<bool>(ReturnBoolProperty);
        if (value == nullptr) {
            return std::unexpected(STR("Return value is null"));
        }
        return *value;
    }

    /** Returns a pointer to the player controller if it exists. */
    std::expected<UObject*, StringType> getPlayerController() {
        static UObject* playerController = nullptr;

        if (playerController == nullptr) {
            playerController = UObjectGlobals::FindFirstOf(STR("PlayerController"));
        }
        if (playerController == nullptr) {
            return std::unexpected(STR("PlayerController not found"));
        }
        return playerController;
    }

    /** Returns a pointer to the player character if it exists. */
    std::expected<UObject*, StringType> getPlayerCharacter() {
        auto playerController = getPlayerController();
        if (!playerController) {
            return std::unexpected(playerController.error());
        }
        auto GetPlayerCharacter = getFunctionByNameInChain(playerController.value(), STR("GetPlayerCharacter"));
        if (!GetPlayerCharacter) {
            return std::unexpected(GetPlayerCharacter.error());
        }
        struct Params {
            UObject* ReturnValue;
        };
        Params params{};
        playerController.value()->ProcessEvent(GetPlayerCharacter.value(), &params);
        if (!params.ReturnValue) {
            return std::unexpected(STR("PlayerCharacter not returned"));
        }
        return params.ReturnValue;
    }

    /** Returns a pointer to the player's movement component if it exists. */
    std::expected<UObject*, StringType> getCharacterMovement() {
        auto playerCharacter = getPlayerCharacter();
        if (!playerCharacter) {
            return std::unexpected(playerCharacter.error());
        }

        UObject** characterMovement =
            playerCharacter.value()->GetValuePtrByPropertyNameInChain<UObject*>(STR("CharacterMovement"));
        if (!characterMovement || !*characterMovement) {
            return std::unexpected(STR("CharacterMovement not found"));
        }
        return *characterMovement;
    }

    /** Returns true if `key` is pressed, otherwise false. */
    std::expected<bool, StringType> isKeyPressed(FKey& key) {
        auto playerController = getPlayerController();
        if (!playerController) {
            return std::unexpected(playerController.error());
        }
        auto IsInputKeyDown = getFunctionByNameInChain(*playerController, STR("IsInputKeyDown"));
        if (!IsInputKeyDown) {
            return std::unexpected(IsInputKeyDown.error());
        }

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
    std::expected<bool, StringType> isKeyPressed(StringType key) {
        FKey fKey = FKey(key);
        return isKeyPressed(fKey);
    }

    /** Returns a pointer to the input mapping context if it exists. */
    std::expected<UObject*, StringType> getInputMappingContext(const StringType& name) {
        static auto nameInputMappingContext = FName(STR("InputMappingContext"));
        auto fName = FName(name);
        auto imc = RC::Unreal::UObjectGlobals::FindObject(nameInputMappingContext, fName);
        if (!imc) {
            return std::unexpected(STR("IMC not found"));
        }
        return imc;
    }

    /** Returns true if the specified input action is pressed, otherwise false. */
    std::expected<bool, StringType> isInputActionKeyPressed(const StringType& inputMappingContextName,
                                                            const StringType& inputActionName) {
        auto imc = getInputMappingContext(inputMappingContextName);
        if (!imc) {
            return std::unexpected(imc.error());
        }

        auto mappingsProperty = imc.value()->GetValuePtrByPropertyNameInChain<FScriptArray>(STR("Mappings"));
        if (!mappingsProperty) {
            return std::unexpected(STR("Property 'Mappings' not found on InputMappingContext"));
        }

        auto mappings = StaticCast<FArrayProperty*>(imc.value()->GetPropertyByNameInChain(STR("Mappings")));
        static auto SprintActionName = FName(inputActionName, FNAME_Add);
        const int32 ElementSize = mappings->GetInner()->GetElementSize();
        auto actionKeyMapping = static_cast<FStructProperty*>(mappings->GetInner());
        FProperty* keyProperty = actionKeyMapping->GetStruct()->GetPropertyByNameInChain(STR("Key"));
        if (!keyProperty) {
            return std::unexpected(STR("Property 'Key' not found on UEnhancedActionKeyMapping"));
        }
        FProperty* actionProperty = actionKeyMapping->GetStruct()->GetPropertyByNameInChain(STR("Action"));
        if (!actionProperty) {
            return std::unexpected(STR("Property 'Action' not found on UEnhancedActionKeyMapping"));
        }
        for (int32_t i = 0; i < mappingsProperty->Num(); i++) {
            const int32 offset = i * ElementSize;
            auto element = static_cast<uint8*>(mappingsProperty->GetData()) + offset;
            auto key = keyProperty->ContainerPtrToValuePtr<FKey>(element);
            auto action = *actionProperty->ContainerPtrToValuePtr<UObject*>(element);

            if (action->GetNamePrivate().Equals(SprintActionName) && isKeyPressed(*key).value_or(false)) {
                return true;
            }
        }
        return false;
    }

    std::expected<UObject*, StringType> getHorse() {
        UObject* playerCharacter = getPlayerCharacter().value();
        auto Function = getFunctionByNameInChain(playerCharacter, STR("GetHorse"));
        if (!Function) {
            return std::unexpected(Function.error());
        }
        FProperty* ReturnProperty = Function.value()->GetReturnProperty();
        void* Params = _malloca(Function.value()->GetParmsSize());
        ReturnProperty->InitializeValue_InContainer(Params);

        playerCharacter->ProcessEvent(Function.value(), Params);
        FObjectProperty* ReturnObjectProperty = ReturnProperty->ContainerPtrToValuePtr<FObjectProperty>(Params);
        UObject** value = ReturnProperty->ContainerPtrToValuePtr<UObject*>(ReturnObjectProperty);
        if (value == nullptr) {
            return std::unexpected(STR("Return value is null"));
        }
        return *value;
    }

    std::expected<UObject*, StringType> getHorseMovement() {
        auto horse = getHorse();
        if (!horse) {
            return std::unexpected(horse.error());
        }

        if (horse.value() == nullptr) {
            return std::unexpected(STR("No horse found."));
        }

        UObject** characterMovement =
            horse.value()->GetValuePtrByPropertyNameInChain<UObject*>(STR("CharacterMovement"));
        if (!characterMovement || !*characterMovement) {
            return std::unexpected(STR("CharacterMovement not found"));
        }
        return *characterMovement;
    }
}
