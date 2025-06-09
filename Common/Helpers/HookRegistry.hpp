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
    using FunctionContext = Unreal::UnrealScriptFunctionCallableContext;
    using UnrealScriptFunctionCallable = Unreal::UnrealScriptFunctionCallable;

    class HookRegistry {
      public:
        std::vector<std::tuple<StringType, int, int>> hookRegistrations;

        ~HookRegistry() {
            for (auto it = hookRegistrations.rbegin(); it != hookRegistrations.rend(); ++it) {
                const auto& [hookName, handle, unregister] = *it;
                if (unregister) {
                    RC::Unreal::UObjectGlobals::UnregisterHook(hookName, {handle, unregister});
                }
            }
        }

        std::pair<int, int> Register(const StringType& functionFullNameNoType, std::function<void()> preCallback) {
            return Register(
                functionFullNameNoType,
                [preCallback](FunctionContext&, void*) { preCallback(); },
                [](FunctionContext&, void*) {},
                nullptr);
        }

        std::pair<int, int> Register(const StringType& functionFullNameNoType,
                                     UnrealScriptFunctionCallable preCallback) {
            return Register(functionFullNameNoType, preCallback, [](FunctionContext&, void*) {}, nullptr);
        }

        std::pair<int, int> Register(const StringType& functionFullNameNoType, std::function<void()> preCallback,
                                     std::function<void()> postCallback) {
            return Register(
                functionFullNameNoType,
                [preCallback](FunctionContext&, void*) { preCallback(); },
                [postCallback](FunctionContext&, void*) { postCallback(); },
                nullptr);
        }

        std::pair<int, int> Register(const StringType& functionFullNameNoType, UnrealScriptFunctionCallable preCallback,
                                     UnrealScriptFunctionCallable postCallback) {
            return Register(functionFullNameNoType, preCallback, postCallback, nullptr);
        }

        std::pair<int, int> Register(const StringType& functionFullNameNoType, UnrealScriptFunctionCallable preCallback,
                                     UnrealScriptFunctionCallable postCallback, void* customData) {
            auto [preId, postId] =
                RC::Unreal::UObjectGlobals::RegisterHook(functionFullNameNoType, preCallback, postCallback, customData);
            hookRegistrations.emplace_back(functionFullNameNoType, preId, postId);
            return {preId, postId};
        }
    };

}
