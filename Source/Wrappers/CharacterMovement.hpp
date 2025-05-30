#pragma once

#include <Helpers/String.hpp>
#include <Unreal/UObject.hpp>

#include "UObject.hpp"


namespace Wrappers {

    class CharacterMovementWrapper : public UObjectWrapper {
      public:
        CharacterMovementWrapper(RC::Unreal::UObject* obj) : UObjectWrapper(obj) {}

        /// Returns true if the character is sprinting.
        bool IsSprinting() const {
            return call<bool>(STR("IsSprinting"));
        }

        /// Returns true if the character is moving on the ground.
        bool IsMovingOnGround() const {
            return call<bool>(STR("IsMovingOnGround"));
        }
    };
}
