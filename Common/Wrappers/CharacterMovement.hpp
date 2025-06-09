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
#include <Unreal/UObject.hpp>

#include "UObject.hpp"


namespace Wrappers {

    class CharacterMovementWrapper : public UObjectWrapper {
      public:
        CharacterMovementWrapper(RC::Unreal::UObject* obj) : UObjectWrapper(obj) {}

        /// Returns true if the character is sprinting.
        bool IsSprinting() const {
            return call<bool>(STR("IsSprinting"));
        }

        /// Returns true if the character is moving on the ground.
        bool IsMovingOnGround() const {
            return call<bool>(STR("IsMovingOnGround"));
        }
    };
}
