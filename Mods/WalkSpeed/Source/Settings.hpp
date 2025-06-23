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

#include <stdio.h>

#include <String/StringType.hpp>
#include <any>
#include <toml++/toml.hpp>

#include "Settings/Settings.hpp"


class Settings : public SettingsBase<bool, float, int, StringType> {
  public:
    Settings() = default;
    Settings(const StringType configPath, const StringType overrideDirectory) : SettingsBase() {
        // Parse the main config.
        readFromFile(configPath);
        // Parse config overrides.
        readFromOverrideDirectory(overrideDirectory);
    }

    const Setting<bool>& HoldToAdjust = AddSetting<bool>(false, "HoldToAdjust");
    const Setting<StringType>& HoldKey = AddSetting<StringType>(STR("LeftAlt"), "HoldKey");
    const Setting<bool>& LockPOV = AddSetting<bool>(true, "LockPOV");
    const Setting<int>& Steps = AddSetting<int>(5, "Steps");
    const Setting<bool>& ScrollTogglesSprintOff = AddSetting<bool>(true, "ScrollTogglesSprintOff");
    const Setting<bool>& ResetSpeedOnRun = AddSetting<bool>(true, "ResetSpeedOnRun");
    const Setting<bool>& ResetSpeedOnSprint = AddSetting<bool>(true, "ResetSpeedOnSprint");

    const Setting<float>& MoveRunMult = AddSetting<float>(3.5f, "MoveRunMult");
    const Setting<float>& MoveRunAthleticsMult = AddSetting<float>(0.75f, "MoveRunAthleticsMult");
    const Setting<float>& MoveSneakRunMult = AddSetting<float>(0.6f, "MoveSneakRunMult");
};
