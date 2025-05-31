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
#include "Metadata.hpp"
#include "MovementController.hpp"
#include "Settings.hpp"
#include "Shapes/FInputActionValue.hpp"
#include "Utils/Logger.hpp"
#include "Utils/ModUtils.hpp"

using namespace ModUtils;


auto modPath = RC::UE4SSProgram::get_program().get_mods_directory();
auto configPath = fmt::format(STR(R"({}\{}\config.toml)"), modPath, MOD_NAME_STR);
auto overrideDirectory = fmt::format(STR(R"({}\{}\ConfigOverrides)"), modPath, MOD_NAME_STR);

class AdvancedMovement : public ModBase {
  public:
    Settings settings;
    CameraController cameraController;
    MovementController mc;

    AdvancedMovement() : settings(configPath, overrideDirectory), mc(settings) {
        ModName = MOD_NAME_STR;
        ModVersion = MOD_VERSION_STR;
        ModDescription = STR("This is my awesome mod");
        ModAuthors = STR("DuckyCrayfish");
    }

    auto on_unreal_init() -> void override {
        RegisterModHook(
            STR("/Script/Altar.VEnhancedAltarPlayerController:MouseWheelUpInput"),
            [](FunctionContext&, void*) {},
            [this](FunctionContext& context, void*) {
                if (!shouldScrollAdjustMovement()) {
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
                    if (pm.IsSprinting() && settings.getScrollTogglesSprintOff()) {
                        pc.ToggleSprint();
                        mc.applyMaxSpeed();
                    } else {
                        mc.decrementSpeed();
                        if (!walking && mc.isAtMinSpeed() && settings.getMoveRunMultMin() == 1) {
                            pc.ToggleWalk();
                        }
                    }
                }
            });

        RegisterModHook(
            STR("/Script/Altar.VEnhancedAltarPlayerController:MouseWheelUpInput"),
            [this]() {
                if (shouldScrollAdjustMovement() && settings.getLockPOV()) {
                    cameraController.lockPOV();
                }
            },
            [this]() { cameraController.resetPOVLock(); });


        if (settings.getResetSpeedOnRun()) {
            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:ToggleWalk"),
                            [this]() { mc.applyMaxSpeed(); });
        }

        if (settings.getResetSpeedOnSprint()) {
            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:ToggleSprint"),
                            [this]() { mc.applyMaxSpeed(); });
        }

        if (settings.getHoldToSprint()) {
            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:MovementForwardInput_Pressed"), [this]() {
                if (!isSprintKeyPressed()) {
                    return;
                }

                auto pc = GetPlayerController();
                auto pm = GetPlayerMovement();

                if (pc.IsHorseRiding()) {
                    if (!pc.GetWantsToGallop()) {
                        pc.ToggleGallop();
                    }
                } else {
                    if (!pm.IsSprinting() && pm.IsMovingOnGround()) {
                        pc.ToggleSprint();
                    }
                }
            });

            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:ShiftKeyInput_Released"), [this]() {
                GetPlayerController().DisableSprintToggle();
                GetPlayerController().DisableGallopToggle();
            });
        }
    }

    /// Returns true if scrolling should currently control movement.
    bool shouldScrollAdjustMovement() {
        return !settings.getHoldToAdjust() || isKeyPressed(settings.getHoldKey()).value();
    }

    /// Returns true if the sprint key is currently pressed, false otherwise.
    bool isSprintKeyPressed() const {
        return isInputActionKeyPressed(STR("IMC_Game_Movement"), STR("IA_Game_Movement_Sprint"));
    }
};

#define MY_AWESOME_MOD_API __declspec(dllexport)
extern "C" {
    MY_AWESOME_MOD_API RC::CppUserModBase* start_mod() {
        return new AdvancedMovement();
    }

    MY_AWESOME_MOD_API void uninstall_mod(RC::CppUserModBase* mod) {
        delete mod;
    }
}
