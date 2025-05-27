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
#include <toml++/toml.hpp>

#include "Utils/Logger.hpp"
#include "Utils/ModUtils.hpp"


struct SettingEntry {
    std::string_view path;
    std::variant<bool *, float *, int *, StringType *> ptr;
};

class Settings {
  private:
    bool useHoldKey = false;
    StringType holdKey = STR("LeftAlt");
    float moveRunMultMin = 1.0f;
    float moveRunMultMax = 3.5f;
    float moveRunAthleticsMultMin = 0.0f;
    float moveRunAthleticsMultMax = 0.75f;
    int steps = 5;
    bool scrollTogglesSprintOff = true;
    bool toggleMaxSpeedOnRun = true;
    bool toggleMaxSpeedOnSprint = true;
    bool holdToSprint = false;

    std::vector<SettingEntry> settingEntries_ = {
        {"settings.useHoldKey", &useHoldKey},
        {"settings.holdKey", &holdKey},
        {"settings.moveRunMultMin", &moveRunMultMin},
        {"settings.moveRunMultMax", &moveRunMultMax},
        {"settings.moveRunAthleticsMultMin", &moveRunAthleticsMultMin},
        {"settings.moveRunAthleticsMultMax", &moveRunAthleticsMultMax},
        {"settings.steps", &steps},
        {"settings.scrollTogglesSprintOff", &scrollTogglesSprintOff},
        {"settings.toggleMaxSpeedOnRun", &toggleMaxSpeedOnRun},
        {"settings.toggleMaxSpeedOnSprint", &toggleMaxSpeedOnSprint},
        {"settings.holdToSprint", &holdToSprint},
    };

  public:
    Settings(const StringType configPath, const StringType overrideDirectory) {
        // Parse the main config.
        parseFile(configPath, false);
        // Parse config overrides.
        for (const auto &entry : std::filesystem::directory_iterator(overrideDirectory)) {
            if (entry.path().extension() == ".toml") {
                auto filePath = entry.path().wstring();
                parseFile(filePath, true);
            }
        }
    }

    bool getUseHoldKey() const {
        return useHoldKey;
    }
    StringType getHoldKey() const {
        return holdKey;
    }
    float getMoveRunMultMin() const {
        return moveRunMultMin;
    }
    float getMoveRunMultMax() const {
        return moveRunMultMax;
    }
    float getMoveRunAthleticsMultMin() const {
        return moveRunAthleticsMultMin;
    }
    float getMoveRunAthleticsMultMax() const {
        return moveRunAthleticsMultMax;
    }
    int getSteps() const {
        return steps;
    }
    bool getScrollTogglesSprintOff() const {
        return scrollTogglesSprintOff;
    }
    bool getToggleMaxSpeedOnRun() const {
        return toggleMaxSpeedOnRun;
    }
    bool getToggleMaxSpeedOnSprint() const {
        return toggleMaxSpeedOnSprint;
    }
    bool getHoldToSprint() const {
        return holdToSprint;
    }

  private:
    /**
     * Parses a config file at `filePath`. If `isOverride` is false, the file will be
     * considered the main config file, otherwise, the file will be considered an
     * override file. This distinction only affects error messages.
     */
    void parseFile(const StringType &filePath, const bool isOverride) {
        toml::table table;
        try {
            table = toml::parse_file(filePath);
        } catch (const toml::parse_error &err) {
            if (!isOverride) {
                Logger::log<LogLevel::Warning>(STR("Failed to parse main config file:\n"));
            } else {
                Logger::log<LogLevel::Warning>(STR("Failed to parse config override file:\n"));
            }

            auto source = to_wstring((std::ostringstream() << err.source().begin).str());
            Logger::log<LogLevel::Warning>(STR("{}\n"), to_wstring(filePath));
            Logger::log<LogLevel::Warning>(STR("Error: {} ({})\n"), to_wstring(err.description()), source);

            if (!isOverride) {
                Logger::log<LogLevel::Warning>(STR("Starting with default values.\n"));
            }
            return;
        }

        if (!isOverride) {
            Logger::log<LogLevel::Verbose>(STR("Found config file:\n"));
            Logger::log<LogLevel::Verbose>(to_wstring(filePath) + STR("\n"));
        } else {
            Logger::log<LogLevel::Normal>(STR("Found config override:\n"));
            Logger::log<LogLevel::Normal>(to_wstring(filePath) + STR("\n"));
        }

        for (const auto &entry : settingEntries_) {
            std::visit(
                [table, entry](auto &&ptr) {
                    using ValueType = std::remove_pointer_t<std::decay_t<decltype(ptr)>>;
                    auto path = toml::path(entry.path);
                    auto node = table[path];
                    if (!node) {
                        // Key does not exist, skip.
                        return;
                    }
                    if (auto value = node.value<ValueType>()) {
                        *ptr = *value;
                    } else {
                        Logger::log<LogLevel::Warning>(STR("Invalid value found at: \"{}\"\n"), to_wstring(entry.path));
                    }
                },
                entry.ptr);
        }
    }
};
