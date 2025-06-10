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

#include "Helpers/UnrealHelpers.hpp"
#include "Logger/Logger.hpp"
#include "Settings.hpp"
#include "Shapes/FInputActionValue.hpp"
#include "Wrappers/Wrappers.hpp"

using namespace Helpers;


/**
 * These variables correspond to run-speed multiplier variables that the game uses to calculate run
 * speed. While these values are unchanging in vanilla, the values here are calibrated to match the
 * standard walking pace. When the mod sets each corresponding variable to these values, walk speed
 * and run speed are equivalent.
 */
namespace WalkSpeedEquivalents {
    /**
     * Walking speed equivalent for MoveRunMult.
     * Game default: 3.5
     */
    inline constexpr float MOVE_RUN_MULT = 1;
    /**
     * Walking speed equivalent for MoveRunAthleticsMult.
     * Game default: 0.75
     */
    inline constexpr float MOVE_RUN_ATHLETICS_MULT = 0;
    /**
     * Walking speed equivalent for MoveSneakRunMult.
     * Game default: 0.6
     */
    inline constexpr float MOVE_SNEAK_RUN_MULT = 1;
}

class MovementController {
  protected:
    /// Mod settings.
    const Settings& settings;
    /// The current speed, as a normalized linear scalar from 0 (walking) to 1 (running).
    float speed = 1;

  private:
    /// Multiplicative growth at each step.
    const float speedStepGain;
    /// Additive growth at each step.
    const float speedStepBias;

    static inline float getSpeedStepGain(float max, int steps) noexcept {
        return std::pow(max, 1.0f / steps);
    }

    static inline float getSpeedStepBias(float max, float gain) noexcept {
        return (WalkSpeedEquivalents::MOVE_RUN_MULT * (gain - 1.0f)) / (max - WalkSpeedEquivalents::MOVE_RUN_MULT);
    }

  public:
    MovementController(const Settings& settings)
        : settings(settings),
          speedStepGain(getSpeedStepGain(settings.moveRunMultMax.get(), settings.steps.get())),
          speedStepBias(getSpeedStepBias(settings.moveRunMultMax.get(), speedStepGain)) {};

    ~MovementController() = default;

    /// Clamps `f` between `min` and `max`. If `f` falls within `epsilon` of either bound, it snaps to that bound.
    static constexpr inline float clampSnap(float f, float min, float max, float epsilon = 1e-3f) noexcept {
        if (f <= min + epsilon) return min;
        if (f >= max - epsilon) return max;
        return f;
    }

    /// Performs a linear interpolation between a and b by t.
    static constexpr float lerp(float a, float b, float t) noexcept {
        return a + t * (b - a);
    }

    /// Increment the speed by one step.
    void incrementSpeed() {
        float newSpeed = std::fma(speedStepGain, speed, speedStepBias);
        newSpeed = clampSnap(newSpeed, 0, 1);
        applySpeed(newSpeed);
    }

    /// Decrement the speed by one step.
    void decrementSpeed() {
        float newSpeed = (speed - speedStepBias) / speedStepGain;
        newSpeed = clampSnap(newSpeed, 0, 1);
        applySpeed(newSpeed);
    }

    /// Set the speed to the minimum value.
    void applyMinSpeed() {
        applySpeed(0);
    }

    /// Set the speed to the maximum value.
    void applyMaxSpeed() {
        applySpeed(1);
    }

    /// Applies a given speed value to the player.
    void applySpeed(float speed) {
        this->speed = speed;
        auto playerMovement = GetPlayerMovement();
        namespace Walking = WalkSpeedEquivalents;
        auto& s = settings;

        // Returns the appropriate multiplier value for the current speed.
        auto setMultValue = [&](const TCHAR* varName, float walkVal, float runVal) {
            float val = lerp(walkVal, runVal, speed);
            playerMovement.SetMemberInChain(varName, val);
        };

        setMultValue(STR("MoveRunMult"), Walking::MOVE_RUN_MULT, s.moveRunMultMax.get());
        setMultValue(STR("MoveRunAthleticsMult"), Walking::MOVE_RUN_ATHLETICS_MULT, s.moveRunAthleticsMultMax.get());
        setMultValue(STR("MoveSneakRunMult"), Walking::MOVE_SNEAK_RUN_MULT, s.sneakSpeedMult.get());
    }

    /// Returns true if the player's current run speed is at the maximum value.
    bool isAtMaxSpeed() const noexcept {
        return speed == 1;
    }

    /// Returns true if the player's current run speed is at the minimum value.
    bool isAtMinSpeed() const noexcept {
        return speed == 0;
    }
};
