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

#define MOD_NAME STR("MyAwesomeMod")

#include <Helpers/String.hpp>
#include <Mod/CppUserModBase.hpp>
#include <UE4SSProgram.hpp>
#include <UnrealDef.hpp>
#include <algorithm>
#include <optional>

#include "CameraController.hpp"
#include "MovementController.hpp"
#include "Settings.hpp"
#include "Shapes/FInputActionValue.hpp"
#include "Utils/Logger.hpp"
#include "Utils/ModUtils.hpp"

using namespace ModUtils;


auto modPath = RC::UE4SSProgram::get_program().get_mods_directory();
auto configPath = fmt::format(STR(R"({}\{}\config.toml)"), modPath, MOD_NAME);
auto overrideDirectory = fmt::format(STR(R"({}\{}\ConfigOverrides)"), modPath, MOD_NAME);

class AdvancedMovement : public ModBase {
  public:
    Settings settings;
    CameraController cameraController;
    MovementController mc;

    AdvancedMovement() : settings(configPath, overrideDirectory), mc(settings) {
        ModName = MOD_NAME;
        ModVersion = STR("1.0");
        ModDescription = STR("This is my awesome mod");
        ModAuthors = STR("DuckyCrayfish");
    }

    auto on_unreal_init() -> void override {
        RegisterModHook(
            STR("/Script/Altar.VEnhancedAltarPlayerController:MouseWheelUpInput"),
            [](FunctionContext&, void*) {},
            [this](FunctionContext& context, void*) {
                bool isScrollUp = context.GetParams<FInputActionValue>().asFloat() > 0;
                auto walking = mc.isWalking();

                if (!shouldScrollAdjustMovement()) {
                    return;
                }

                if (isScrollUp) {
                    if (walking) {
                        mc.toggleWalk();
                        mc.applyMinSpeed();
                    }
                    mc.incrementSpeed();
                } else {
                    if (mc.isSprinting() && settings.getScrollTogglesSprintOff()) {
                        mc.toggleSprint();
                        mc.applyMaxSpeed();
                    } else {
                        mc.decrementSpeed();
                        if (!walking && mc.isAtMinSpeed() && settings.getMoveRunMultMin() == 1) {
                            mc.toggleWalk();
                        }
                    }
                }
            });

        RegisterModHook(
            STR("/Script/Altar.VEnhancedAltarPlayerController:MouseWheelUpInput"),
            [this]() {
                if (shouldScrollAdjustMovement()) {
                    cameraController.lockPOV();
                }
            },
            [this]() { cameraController.resetPOVLock(); });


        if (settings.getToggleMaxSpeedOnRun()) {
            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:ToggleWalk"),
                            [this]() { mc.applyMaxSpeed(); });
        }

        if (settings.getToggleMaxSpeedOnSprint()) {
            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:ToggleSprint"),
                            [this]() { mc.applyMaxSpeed(); });
        }

        if (settings.getHoldToSprint()) {
            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:MovementForwardInput_Pressed"), [this]() {
                if (!mc.isSprintKeyPressed()) {
                    return;
                }


                if (getHorse().value()) {
                    if (!mc.wantsToGallop()) {
                        mc.toggleGallop();
                    }
                } else {
                    if (!mc.isSprinting() && mc.isMovingOnGround()) {
                        mc.toggleSprint();
                    }
                }
            });

            RegisterModHook(STR("/Script/Altar.VEnhancedAltarPlayerController:ShiftKeyInput_Released"), [this]() {
                mc.disableSprintToggle();
                mc.disableGallopToggle();
            });
        }
    }

    bool shouldScrollAdjustMovement() {
        return !settings.getUseHoldKey() || isKeyPressed(settings.getHoldKey()).value();
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
