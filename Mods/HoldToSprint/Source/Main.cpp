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

#include "Helpers/HookRegistry.hpp"
#include "Helpers/UnrealHelpers.hpp"
#include "Logger/Logger.hpp"
#include "Settings.hpp"

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

class HoldToSprint : public RC::CppUserModBase {
  public:
    Settings settings;
    Helpers::HookRegistry hooks;

    HoldToSprint() : settings(configPath, overrideDirectory) {
        ModName = MOD_NAME_STR;
        ModVersion = MOD_VERSION_STR;
        ModDescription = STR("This is my awesome mod");
        ModAuthors = STR("DuckyCrayfish");
    }

    auto on_unreal_init() -> void override {
        hooks.Register(STR("/Script/Altar.VEnhancedAltarPlayerController:MovementForwardInput_Pressed"), [this]() {
            if (settings.holdToSprint.get()) {
                if (!IsSprintKeyPressed()) {
                    return;
                }
                auto pc = Helpers::GetPlayerController();
                auto pm = Helpers::GetPlayerMovement();
                if (pc.IsHorseRiding()) {
                    if (!pc.GetWantsToGallop()) {
                        pc.ToggleGallop();
                    }
                } else {
                    if (!pm.IsSprinting() && pm.IsMovingOnGround()) {
                        pc.ToggleSprint();
                    }
                }
            }
        });

        hooks.Register(STR("/Script/Altar.VEnhancedAltarPlayerController:ShiftKeyInput_Released"), [this]() {
            auto pc = Helpers::GetPlayerController();
            if (settings.holdToSprint.get()) {
                pc.DisableSprintToggle();
            }
            pc.DisableGallopToggle();
        });
    }

  private:
    /// Returns true if the sprint key is currently pressed, false otherwise.
    bool IsSprintKeyPressed() const {
        return Helpers::IsInputActionKeyPressed(STR("IMC_Game_Movement"), STR("IA_Game_Movement_Sprint"));
    }
};

#define DYNAMIC_MOVEMENT_HOLD_TO_SPRINT_API __declspec(dllexport)
extern "C" {
    DYNAMIC_MOVEMENT_HOLD_TO_SPRINT_API RC::CppUserModBase* start_mod() {
        return new HoldToSprint();
    }

    DYNAMIC_MOVEMENT_HOLD_TO_SPRINT_API void uninstall_mod(RC::CppUserModBase* mod) {
        delete mod;
    }
}
