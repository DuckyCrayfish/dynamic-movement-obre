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

#include <Mod/CppUserModBase.hpp>
#include <UnrealDef.hpp>
#include <algorithm>
#include <optional>

#include "Settings.hpp"
#include "Shapes/FInputActionValue.hpp"
#include "Utils/Logger.hpp"
#include "Utils/ModUtils.hpp"

using namespace RC::Unreal;
using namespace ModUtils;


class MovementController {
  protected:
    /// Mod settings.
    const Settings& settings;
    /// The current run speed multiplier.
    float moveRunMult_;
    /// The current run speed athletics multiplier.
    float moveRunAthleticsMult_;

  private:
    const float positiveStepScalar;
    const float negativeStepScalar;
    float calculateSpeedScalar(const float from, const float to) const {
        return std::pow(to / from, 1.0f / settings.getSteps());
    }

  public:
    MovementController(const Settings& settings_)
        : settings(settings_),
          moveRunMult_(settings_.getMoveRunMultMax()),
          moveRunAthleticsMult_(settings_.getMoveRunAthleticsMultMax()),
          positiveStepScalar(calculateSpeedScalar(settings.getMoveRunMultMin(), settings.getMoveRunMultMax())),
          negativeStepScalar(calculateSpeedScalar(settings.getMoveRunMultMax(), settings.getMoveRunMultMin())) {};

    ~MovementController() = default;

    /// Returns true if a and b are equal or nearly equal, false otherwise.
    bool nearlyEqual(float a, float b, float epsilon = 1e-3f) const {
        return std::fabs(a - b) < epsilon;
    }

    /// Performs a linear interpolation between a and b by t.
    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    /// Performs an inverse linear interpolation between a and b by t.
    float inverse_lerp(float a, float b, float t) const {
        return (t - a) / (b - a);
    }

    /// Returns the inverse lerp of the given run speed multiplier between min and max, mapping it to [0, 1].
    float getNormalizedSpeedFactor(float moveRunMult) const {
        float value = std::clamp(
            inverse_lerp(settings.getMoveRunMultMin(), settings.getMoveRunMultMax(), moveRunMult), 0.0f, 1.0f);
        if (nearlyEqual(value, 0.0f)) {
            return 0.0f;
        } else if (nearlyEqual(value, 1.0f)) {
            return 1.0f;
        }
        return value;
    }

    /// Increment the speed by one step.
    void incrementSpeed() {
        float speedFactor = getNormalizedSpeedFactor(moveRunMult_ * positiveStepScalar);
        moveRunMult_ = lerp(settings.getMoveRunMultMin(), settings.getMoveRunMultMax(), speedFactor);
        moveRunAthleticsMult_ =
            lerp(settings.getMoveRunAthleticsMultMin(), settings.getMoveRunAthleticsMultMax(), speedFactor);
        applySpeed();
    }

    /// Decrement the speed by one step.
    void decrementSpeed() {
        float speedFactor = getNormalizedSpeedFactor(moveRunMult_ * negativeStepScalar);
        moveRunMult_ = lerp(settings.getMoveRunMultMin(), settings.getMoveRunMultMax(), speedFactor);
        moveRunAthleticsMult_ =
            lerp(settings.getMoveRunAthleticsMultMin(), settings.getMoveRunAthleticsMultMax(), speedFactor);
        applySpeed();
    }

    /// Set the speed to the minimum value.
    void applyMinSpeed() {
        moveRunMult_ = settings.getMoveRunMultMin();
        moveRunAthleticsMult_ = settings.getMoveRunAthleticsMultMin();
        applySpeed();
    }

    /// Set the speed to the maximum value.
    void applyMaxSpeed() {
        moveRunMult_ = settings.getMoveRunMultMax();
        moveRunAthleticsMult_ = settings.getMoveRunAthleticsMultMax();
        applySpeed();
    }

    /// Apply the speed values currently stored in this instance to the player.
    void applySpeed() {
        auto characterMovement = getCharacterMovement();
        if (!characterMovement) {
            return;
        }
        auto MoveRunMult = characterMovement.value()->GetValuePtrByPropertyNameInChain<float>(STR("MoveRunMult"));
        auto MoveRunAthleticsMult =
            characterMovement.value()->GetValuePtrByPropertyNameInChain<float>(STR("MoveRunAthleticsMult"));
        if (!MoveRunMult) {
            Logger::log<LogLevel::Error>(STR("Could not apply speed: null property MoveRunMult\n"));
            return;
        }
        if (!MoveRunAthleticsMult) {
            Logger::log<LogLevel::Error>(STR("Could not apply speed: null property MoveRunAthleticsMult\n"));
            return;
        }
        *MoveRunMult = moveRunMult_;
        *MoveRunAthleticsMult = moveRunAthleticsMult_;
    }

    /// Returns true if the player's current run speed is at the maximum value.
    bool isAtMaxSpeed() const {
        return moveRunMult_ == settings.getMoveRunMultMax();
    }

    /// Returns true if the player's current run speed is at the minimum value.
    bool isAtMinSpeed() const {
        return moveRunMult_ == settings.getMoveRunMultMin();
    }

    /// Returns true if the player is walking, false otherwise.
    bool isWalking() const {
        UObject* playerController = getPlayerController().value();
        return callPropositional(playerController, STR("IsWalking")).value();
    }

    /// Returns true if the character is sprinting, false otherwise.
    bool isSprinting() const {
        UObject* characterMovement = getCharacterMovement().value();
        return callPropositional(characterMovement, STR("IsSprinting")).value();
    }

    /// Returns true if the character is currently moving on the ground, false otherwise.
    bool isMovingOnGround() const {
        UObject* characterMovement = getCharacterMovement().value();
        return callPropositional(characterMovement, STR("IsMovingOnGround")).value();
    }

    /// Returns true if the sprint key is currently pressed, false otherwise.
    bool isSprintKeyPressed() {
        return isInputActionKeyPressed(STR("IMC_Game_Movement"), STR("IA_Game_Movement_Sprint")).value();
    }

    /// Toggles between walking and running.
    void toggleWalk() const {
        UObject* playerController = getPlayerController().value();
        UFunction* ToggleWalk = getFunctionByNameInChain(playerController, STR("ToggleWalk")).value();
        auto params = FInputActionValue::Create(true);
        playerController->ProcessEvent(ToggleWalk, &params);
    }

    /// Toggles sprint on and off.
    void toggleSprint() const {
        UObject* playerController = getPlayerController().value();
        UFunction* ToggleSprint = getFunctionByNameInChain(playerController, STR("ToggleSprint")).value();
        auto params = FInputActionValue::Create(true);
        playerController->ProcessEvent(ToggleSprint, &params);
    }

    bool wantsToGallop() const {
        UObject* playerController = getPlayerController().value();
        return callPropositional(playerController, STR("GetWantsToGallop")).value();
    }

    /// Toggles gallop on and off.
    void toggleGallop() const {
        UObject* playerController = getPlayerController().value();
        UFunction* ToggleGallop = getFunctionByNameInChain(playerController, STR("ToggleGallop")).value();
        auto params = FInputActionValue::Create(true);
        playerController->ProcessEvent(ToggleGallop, &params);
    }

    /// Not quite sure what this does exactly but it seems to just disable sprinting.
    void disableSprintToggle() const {
        UObject* playerController = getPlayerController().value();
        UFunction* DisableSprintToggle = getFunctionByNameInChain(playerController, STR("DisableSprintToggle")).value();
        playerController->ProcessEvent(DisableSprintToggle, nullptr);
    }

    void disableGallopToggle() const {
        UObject* playerController = getPlayerController().value();
        UFunction* DisableGallopToggle = getFunctionByNameInChain(playerController, STR("DisableGallopToggle")).value();
        playerController->ProcessEvent(DisableGallopToggle, nullptr);
    }
};
