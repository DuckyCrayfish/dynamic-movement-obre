#include "PlayerCharacter.hpp"

#include <Helpers/String.hpp>
#include <UnrealDef.hpp>

#include "CharacterMovement.hpp"

using namespace RC;
using namespace RC::Unreal;


namespace Wrappers {

    CharacterMovementWrapper PlayerCharacterWrapper::GetCharacterMovement() {
        UObject** characterMovement = wrapped->GetValuePtrByPropertyNameInChain<UObject*>(STR("CharacterMovement"));
        if (!characterMovement || !*characterMovement) {
            throw std::runtime_error("CharacterMovement not found");
        }
        return CharacterMovementWrapper(*characterMovement);
    }

}
