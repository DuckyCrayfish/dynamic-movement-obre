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
            throw std::runtime_error(to_string(fmt::format(STR("Function {} not found on object"), functionName)));
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
