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

/**
 * @file
 * @brief Logging utilities.
 *
 * @note You must define MOD_NAME (as a wide string, e.g. STR("MyMod")) before including this header.
 */

#pragma once

#include <DynamicOutput/DynamicOutput.hpp>
#include <UnrealDef.hpp>

#ifndef MOD_NAME
#error "MOD_NAME must be defined."
#endif


namespace Logger {
    /// Wrapper for `log` that prepends the mod name to the message.
    template <typename... FmtArgs>
    void log(RC::StringViewType content, FmtArgs... fmt_args) {
        RC::Output::send(std::format(STR("[{}] {}"), MOD_NAME, RC::to_wstring(content)), fmt_args...);
    }

    /// Wrapper for `log` that prepends the mod name to the message.
    template <RC::Output::EnumType OptionalArg, typename... FmtArgs>
    void log(RC::StringViewType content, OptionalArg optional_arg, FmtArgs... fmt_args) {
        RC::Output::send<optional_arg>(std::format(STR("[{}] {}"), MOD_NAME, RC::to_wstring(content)), fmt_args...);
    }

    /// Wrapper for `log` that prepends the mod name to the message.
    template <RC::Output::EnumType OptionalArg>
    void log(RC::StringViewType content, OptionalArg optional_arg) {
        RC::Output::send<optional_arg>(std::format(STR("[{}] {}"), MOD_NAME, RC::to_wstring(content)));
    }

    /// Wrapper for `log` that prepends the mod name to the message.
    template <int32_t optional_arg, typename... FmtArgs>
    void log(RC::StringViewType content, FmtArgs... fmt_args) {
        RC::Output::send<optional_arg>(std::format(STR("[{}] {}"), MOD_NAME, RC::to_wstring(content)), fmt_args...);
    }

    /// Wrapper for `log` that prepends the mod name to the message.
    template <int32_t optional_arg>
    void log(RC::StringViewType content) {
        RC::Output::send<optional_arg>(std::format(STR("[{}] {}"), MOD_NAME, RC::to_wstring(content)));
    }
}
