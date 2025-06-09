#pragma once

#include <Unreal/NameTypes.hpp>


struct FKey {
    FName KeyName{};
    void* KeyDetails{};

    FKey() = default;
    FKey(FName keyName) : KeyName(keyName) {}
    FKey(StringViewType keyName) : KeyName(FName(keyName)) {}
};
