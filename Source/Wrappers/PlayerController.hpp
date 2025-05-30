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

        PlayerCharacterWrapper GetPlayerCharacter();


        /* Oblivion Specific Functions */

        bool IsWalking() const {
            return call<bool>(STR("IsWalking"));
        }

        bool IsHorseRiding() const {
            return call<bool>(STR("IsHorseRiding"));
        }

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

        void DisableSprintToggle() const {
            call(STR("DisableSprintToggle"), nullptr);
        }

        void DisableGallopToggle() const {
            call(STR("DisableGallopToggle"), nullptr);
        }
    };

}
