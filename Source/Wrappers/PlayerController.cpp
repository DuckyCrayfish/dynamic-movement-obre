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

#include "PlayerController.hpp"

#include <Helpers/String.hpp>
#include <UnrealDef.hpp>

#include "PlayerCharacter.hpp"

using namespace RC;
using namespace RC::Unreal;


namespace Wrappers {

    PlayerCharacterWrapper PlayerControllerWrapper::GetPlayerCharacter() {
        auto GetPlayerCharacter = GetFunctionByNameInChain(STR("GetPlayerCharacter"));
        struct Params {
            UObject* ReturnValue;
        };
        Params params{};
        wrapped->ProcessEvent(GetPlayerCharacter.value(), &params);
        if (!params.ReturnValue) {
            throw std::runtime_error("PlayerCharacter not found");
        }
        return PlayerCharacterWrapper(params.ReturnValue);
    }

}
