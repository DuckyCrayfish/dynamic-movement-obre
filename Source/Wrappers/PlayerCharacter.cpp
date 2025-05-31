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

#include "PlayerCharacter.hpp"

#include <Helpers/String.hpp>
#include <UnrealDef.hpp>

#include "CharacterMovement.hpp"

using namespace RC;
using namespace RC::Unreal;


namespace Wrappers {

    CharacterMovementWrapper PlayerCharacterWrapper::GetCharacterMovement() {
        UObject** characterMovement = wrapped->GetValuePtrByPropertyNameInChain<UObject*>(STR("CharacterMovement"));
        if (!characterMovement || !*characterMovement) {
            throw std::runtime_error("CharacterMovement not found");
        }
        return CharacterMovementWrapper(*characterMovement);
    }

}
