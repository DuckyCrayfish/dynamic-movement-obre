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

#include <Helpers/String.hpp>
#include <Mod/CppUserModBase.hpp>
#include <UE4SSProgram.hpp>
#include <UnrealDef.hpp>
#include <algorithm>
#include <optional>

#include "CameraController.hpp"
#include "Helpers/HookRegistry.hpp"
#include "Helpers/UnrealHelpers.hpp"
#include "Logger/Logger.hpp"
#include "MovementController.hpp"
#include "Settings.hpp"
#include "Shapes/FInputActionValue.hpp"

// Set using build system variables
#ifndef MOD_NAME
#error "MOD_NAME must be defined."
#endif

#ifndef MOD_VERSION
#error "MOD_VERSION must be defined."
#endif

#define STR_MACRO(x) STR(x)
#define MOD_NAME_STR STR_MACRO(MOD_NAME)
#define MOD_VERSION_STR STR_MACRO(MOD_VERSION)


auto modPath = RC::UE4SSProgram::get_program().get_mods_directory();
auto configPath = fmt::format(STR(R"({}\{}\config.toml)"), modPath, MOD_NAME_STR);
auto overrideDirectory = fmt::format(STR(R"({}\{}\ConfigOverrides)"), modPath, MOD_NAME_STR);

class WalkSpeed : public RC::CppUserModBase {
  public:
    Settings settings;
    CameraController cameraController;
    MovementController mc;
    Helpers::HookRegistry hooks;

    WalkSpeed() : settings(configPath, overrideDirectory), mc(settings) {
        ModName = MOD_NAME_STR;
        ModVersion = MOD_VERSION_STR;
        ModDescription = STR("This is my awesome mod");
        ModAuthors = STR("DuckyCrayfish");
    }

    auto on_unreal_init() -> void override {
        hooks.Register(
            STR("/Script/Altar.VEnhancedAltarPlayerController:MouseWheelUpInput"),
            [](FunctionContext&, void*) {},
            [this](FunctionContext& context, void*) {
                if (!IsControllingSpeed()) {
                    return;
                }

                bool isScrollUp = context.GetParams<FInputActionValue>().asFloat() > 0;
                auto pc = GetPlayerController();
                auto pm = GetPlayerMovement();
                auto walking = pc.IsWalking();

                if (isScrollUp) {
                    if (walking) {
                        pc.ToggleWalk();
                        mc.applyMinSpeed();
                    }
                    mc.incrementSpeed();
                } else {
                    if (pm.IsSprinting() && settings.ScrollTogglesSprintOff.get()) {
                        pc.ToggleSprint();
                        mc.applyMaxSpeed();
                    } else {
                        mc.decrementSpeed();
                        if (!walking && mc.isAtMinSpeed()) {
                            pc.ToggleWalk();
                        }
                    }
                }
            });

        hooks.Register(
            STR("/Script/Altar.VEnhancedAltarPlayerController:MouseWheelUpInput"),
            [this]() {
                if (IsControllingSpeed() && settings.LockPOV.get()) {
                    cameraController.lockPOV();
                }
            },
            [this]() { cameraController.resetPOVLock(); });


        if (settings.ResetSpeedOnRun.get()) {
            hooks.Register(STR("/Script/Altar.VEnhancedAltarPlayerController:ToggleWalk"),
                           [this]() { mc.applyMaxSpeed(); });
        }

        if (settings.ResetSpeedOnSprint.get()) {
            hooks.Register(STR("/Script/Altar.VEnhancedAltarPlayerController:ToggleSprint"),
                           [this]() { mc.applyMaxSpeed(); });
        }
    }

    /// Returns true if scrolling should currently control movement.
    bool IsControllingSpeed() {
        return !settings.HoldToAdjust.get() || Helpers::IsKeyPressed(settings.HoldKey.get()).value();
    }
};

#define DYNAMIC_MOVEMENT_WALK_SPEED_API __declspec(dllexport)
extern "C" {
    DYNAMIC_MOVEMENT_WALK_SPEED_API RC::CppUserModBase* start_mod() {
        return new WalkSpeed();
    }

    DYNAMIC_MOVEMENT_WALK_SPEED_API void uninstall_mod(RC::CppUserModBase* mod) {
        delete mod;
    }
}
