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

#include "../Shapes/FInputActionValue.hpp"
#include "PlayerCharacter.hpp"
#include "UObject.hpp"


namespace Wrappers {

    class PlayerControllerWrapper : public UObjectWrapper {
      public:
        PlayerControllerWrapper(RC::Unreal::UObject* obj) : UObjectWrapper(obj) {}

        /// Returns the player character associated with this PlayerController.
        PlayerCharacterWrapper GetPlayerCharacter();


        // Oblivion Specific Functions

        /// Whether or not the player's character is in Oblivion's "walk" mode or not.
        bool IsWalking() const {
            return call<bool>(STR("IsWalking"));
        }

        /// Whether or not the player's character is on a horse.
        bool IsHorseRiding() const {
            return call<bool>(STR("IsHorseRiding"));
        }

        /// Basically checks if galloping is toggled on or not.
        /// Analogous to `IsSprinting` for movement on foot.
        bool GetWantsToGallop() const {
            return call<bool>(STR("GetWantsToGallop"));
        }

        /// Toggles between walking and running.
        void ToggleWalk() const {
            auto Value = FInputActionValue::Create(true);
            ToggleWalk(Value);
        }
        void ToggleWalk(FInputActionValue& Value) const {
            call(STR("ToggleWalk"), &Value);
        }

        /// Toggles sprint on and off.
        void ToggleSprint() const {
            auto Value = FInputActionValue::Create(true);
            ToggleSprint(Value);
        }
        void ToggleSprint(FInputActionValue& Value) const {
            call(STR("ToggleSprint"), &Value);
        }

        /// Toggles gallop on and off.
        void ToggleGallop() const {
            auto Value = FInputActionValue::Create(true);
            ToggleGallop(Value);
        }
        void ToggleGallop(FInputActionValue& Value) const {
            call(STR("ToggleGallop"), &Value);
        }

        /// I believe this indicates that the next key-up should disable sprinting.
        void DisableSprintToggle() const {
            call(STR("DisableSprintToggle"), nullptr);
        }

        /// I believe this indicates that the next key-up should disable galloping.
        void DisableGallopToggle() const {
            call(STR("DisableGallopToggle"), nullptr);
        }
    };

}
