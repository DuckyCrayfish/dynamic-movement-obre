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

#include "Utils/Logger.hpp"
#include "Utils/ModUtils.hpp"


template <typename T>
class Setting {
  private:
    friend class Settings;
    T value;

  public:
    const std::string_view path;

    Setting(T defaultValue, std::string_view path) : value(defaultValue), path(path) {}

    T get() const {
        return value;
    }
};


class Settings {
  private:
    std::vector<std::variant<Setting<bool>*, Setting<int>*, Setting<float>*, Setting<StringType>*>> entries;

  public:
    Setting<bool>& holdToAdjust = AddSetting<bool>(true, "settings.holdToAdjust");
    Setting<StringType>& holdKey = AddSetting<StringType>(STR("LeftAlt"), "settings.holdKey");
    Setting<bool>& lockPOV = AddSetting<bool>(true, "settings.lockPOV");
    Setting<int>& steps = AddSetting<int>(5, "settings.steps");
    Setting<bool>& scrollTogglesSprintOff = AddSetting<bool>(true, "settings.scrollTogglesSprintOff");
    Setting<bool>& resetSpeedOnRun = AddSetting<bool>(true, "settings.resetSpeedOnRun");
    Setting<bool>& resetSpeedOnSprint = AddSetting<bool>(true, "settings.resetSpeedOnSprint");
    Setting<float>& moveRunMultMin = AddSetting<float>(1.0f, "settings.moveRunMultMin");
    Setting<float>& moveRunMultMax = AddSetting<float>(3.5f, "settings.moveRunMultMax");
    Setting<float>& moveRunAthleticsMultMin = AddSetting<float>(0.0f, "settings.moveRunAthleticsMultMin");
    Setting<float>& moveRunAthleticsMultMax = AddSetting<float>(0.75f, "settings.moveRunAthleticsMultMax");
    Setting<float>& sneakSpeedMult = AddSetting<float>(0.6f, "settings.sneakSpeedMult");
    Setting<bool>& holdToSprint = AddSetting<bool>(true, "settings.holdToSprint");

    Settings(const StringType configPath, const StringType overrideDirectory) {
        // Parse the main config.
        parseFile(configPath, false);
        // Parse config overrides.
        for (const auto& entry : std::filesystem::directory_iterator(overrideDirectory)) {
            if (entry.path().extension() == ".toml") {
                auto filePath = entry.path().wstring();
                parseFile(filePath, true);
            }
        }
    }

    template <typename T>
    Setting<T>& AddSetting(T defaultValue, std::string_view path) {
        auto* entry = new Setting<T>(defaultValue, path);
        entries.emplace_back(entry);
        return *entry;
    }

  private:
    /**
     * Parses a config file at `filePath`. If `isOverride` is false, the file will be
     * considered the main config file, otherwise, the file will be considered an
     * override file. This distinction only affects error messages.
     */
    void parseFile(const StringType& filePath, const bool isOverride) {
        toml::table table;
        try {
            table = toml::parse_file(filePath);
        } catch (const toml::parse_error& err) {
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

        for (auto& entry : entries) {
            std::visit(
                [table](auto* entry) {
                    using ValueType = std::decay_t<decltype(entry->value)>;
                    auto path = toml::path(entry->path);
                    auto node = table[path];
                    if (!node) {
                        // Key does not exist, skip.
                        return;
                    }
                    if (auto value = node.value<ValueType>()) {
                        entry->value = *value;
                    } else {
                        Logger::log<LogLevel::Warning>(STR("Invalid value found at: \"{}\"\n"),
                                                       to_wstring(entry->path));
                    }
                },
                entry);
        }
    }
};
