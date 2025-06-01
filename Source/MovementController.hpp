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

#include <algorithm>

#include "Settings.hpp"
#include "Shapes/FInputActionValue.hpp"
#include "Utils/Logger.hpp"
#include "Utils/ModUtils.hpp"
#include "Wrappers/Wrappers.hpp"

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
        return std::pow(to / from, 1.0f / settings.steps.get());
    }

  public:
    MovementController(const Settings& settings_)
        : settings(settings_),
          moveRunMult_(settings_.moveRunMultMax.get()),
          moveRunAthleticsMult_(settings_.moveRunAthleticsMultMax.get()),
          positiveStepScalar(calculateSpeedScalar(settings.moveRunMultMin.get(), settings.moveRunMultMax.get())),
          negativeStepScalar(calculateSpeedScalar(settings.moveRunMultMax.get(), settings.moveRunMultMin.get())) {};

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
        float value =
            std::clamp(inverse_lerp(settings.moveRunMultMin.get(), settings.moveRunMultMax.get(), moveRunMult),
                       0.0f,
                       1.0f);
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
        moveRunMult_ = lerp(settings.moveRunMultMin.get(), settings.moveRunMultMax.get(), speedFactor);
        moveRunAthleticsMult_ =
            lerp(settings.moveRunAthleticsMultMin.get(), settings.moveRunAthleticsMultMax.get(), speedFactor);
        applySpeed();
    }

    /// Decrement the speed by one step.
    void decrementSpeed() {
        float speedFactor = getNormalizedSpeedFactor(moveRunMult_ * negativeStepScalar);
        moveRunMult_ = lerp(settings.moveRunMultMin.get(), settings.moveRunMultMax.get(), speedFactor);
        moveRunAthleticsMult_ =
            lerp(settings.moveRunAthleticsMultMin.get(), settings.moveRunAthleticsMultMax.get(), speedFactor);
        applySpeed();
    }

    /// Set the speed to the minimum value.
    void applyMinSpeed() {
        moveRunMult_ = settings.moveRunMultMin.get();
        moveRunAthleticsMult_ = settings.moveRunAthleticsMultMin.get();
        applySpeed();
    }

    /// Set the speed to the maximum value.
    void applyMaxSpeed() {
        moveRunMult_ = settings.moveRunMultMax.get();
        moveRunAthleticsMult_ = settings.moveRunAthleticsMultMax.get();
        applySpeed();
    }

    /// Apply the speed values currently stored in this instance to the player.
    void applySpeed() const {
        auto playerMovement = GetPlayerMovement();
        float sneakSpeedFactor = inverse_lerp(1, settings.moveRunMultMax.get(), moveRunMult_);
        float sneakSpeed = lerp(1, settings.sneakSpeedMult.get(), sneakSpeedFactor);
        playerMovement.SetMemberInChain(STR("MoveRunMult"), moveRunMult_);
        playerMovement.SetMemberInChain(STR("MoveRunAthleticsMult"), moveRunAthleticsMult_);
        playerMovement.SetMemberInChain(STR("MoveSneakRunMult"), sneakSpeed);
    }

    /// Returns true if the player's current run speed is at the maximum value.
    bool isAtMaxSpeed() const {
        return moveRunMult_ == settings.moveRunMultMax.get();
    }

    /// Returns true if the player's current run speed is at the minimum value.
    bool isAtMinSpeed() const {
        return moveRunMult_ == settings.moveRunMultMin.get();
    }
};
