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

#include "UObject.hpp"

#include <Helpers/String.hpp>
#include <UnrealDef.hpp>

#include "../Utils/Logger.hpp"

using namespace RC;
using namespace RC::Unreal;


namespace Wrappers {

    UFunctionWrapper UObjectWrapper::GetFunctionByName(const TCHAR* functionName) const {
        UFunction* function = wrapped->GetFunctionByName(functionName);
        if (function == nullptr) {
            throw std::runtime_error(to_string(
                fmt::format(STR("Function {} not found on object {}"), functionName, wrapped->GetFullName())));
        }
        return UFunctionWrapper(function);
    }

    UFunctionWrapper UObjectWrapper::GetFunctionByNameInChain(const TCHAR* functionName) const {
        UFunction* function = wrapped->GetFunctionByNameInChain(functionName);
        if (function == nullptr) {
            throw std::runtime_error(to_string(
                fmt::format(STR("Function {} not found on object {}"), functionName, wrapped->GetFullName())));
        }

        return UFunctionWrapper(function);
    }

    void UObjectWrapper::PrintAllPropertyNames() {
        auto WrappedClass = wrapped->GetClassPrivate();

        Logger::log<LogLevel::Verbose>(STR("Properties:\n"));
        for (UStruct* Struct = WrappedClass; Struct != nullptr; Struct = Struct->GetSuperStruct()) {
            for (RC::Unreal::FProperty* Property = Struct->GetPropertyLink(); Property != nullptr;
                 Property = Property->GetPropertyLinkNext()) {
                Logger::log<LogLevel::Verbose>(STR("  {}\n"), Property->GetName());
            }
        }
    }

    void UObjectWrapper::PrintAllFunctionNames() {
        auto WrappedClass = wrapped->GetClassPrivate();

        Logger::log<LogLevel::Verbose>(STR("Functions:\n"));
        for (UStruct* Struct = WrappedClass; Struct != nullptr; Struct = Struct->GetSuperStruct()) {
            for (UField* Field = Struct->GetChildren(); Field != nullptr; Field = Field->GetNext()) {
                if (UFunction* Function = Cast<UFunction>(Field)) {
                    Logger::log<LogLevel::Verbose>(STR("  {}\n"), Function->GetName());
                }
            }
        }
    }

    void UObjectWrapper::PrintAllChildren() {
        PrintAllPropertyNames();
        Logger::log<LogLevel::Verbose>(STR("\n"));
        PrintAllFunctionNames();
    }
}
