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

#include "Logger/Logger.hpp"

template <typename T>
class Setting {
  private:
    template <typename... Ts>
    friend class SettingsBase;
    T value;

  public:
    const std::string_view path;

    Setting(T defaultValue, std::string_view path) : value(defaultValue), path(path) {}

    T get() const {
        return value;
    }
};

template <typename... Ts>
class SettingsBase {
  public:
    SettingsBase() {}

  protected:
    std::vector<std::variant<Setting<Ts>*...>> entries;

    template <typename T>
    Setting<T>& AddSetting(T defaultValue, std::string_view path) {
        Logger::log(STR("ADDING\n"));
        auto* entry = new Setting<T>(defaultValue, path);
        entries.emplace_back(entry);
        return *entry;
    }

    /**
     * Parses a config file at `filePath`. If `isOverride` is false, the file will be
     * considered the main config file, otherwise, the file will be considered an
     * override file. This distinction only affects error messages.
     */
    void readFromFile(const StringType& filePath, const bool isOverride = false) {
        toml::table table;
        try {
            table = toml::parse_file(filePath);
            if (!isOverride) {
                Logger::log<LogLevel::Verbose>(STR("Found config file:\n"));
                Logger::log<LogLevel::Verbose>(to_wstring(filePath) + STR("\n"));
            } else {
                Logger::log<LogLevel::Normal>(STR("Found config override:\n"));
                Logger::log<LogLevel::Normal>(to_wstring(filePath) + STR("\n"));
            }
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

        Logger::log(STR("Reading\n"));
        Logger::log(STR("Entries: {}\n"), entries.size());
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
                        Logger::log(STR("Set"));
                        entry->value = *value;
                    } else {
                        Logger::log<LogLevel::Warning>(STR("Invalid value found at: \"{}\"\n"),
                                                       to_wstring(entry->path));
                    }
                },
                entry);
        }
    }

    void readFromOverrideDirectory(const StringType& directoryPath) {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.path().extension() == ".toml") {
                auto filePath = entry.path().wstring();
                readFromFile(filePath, true);
            }
        }
    }
};
