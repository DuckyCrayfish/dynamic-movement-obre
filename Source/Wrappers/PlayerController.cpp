#include "PlayerController.hpp"

#include <Helpers/String.hpp>
#include <UnrealDef.hpp>

#include "PlayerCharacter.hpp"

using namespace RC;
using namespace RC::Unreal;


namespace Wrappers {

    PlayerCharacterWrapper PlayerControllerWrapper::GetPlayerCharacter() {
        auto GetPlayerCharacter = GetFunctionByNameInChain(STR("GetPlayerCharacter"));
        struct Params {
            UObject* ReturnValue;
        };
        Params params{};
        wrapped->ProcessEvent(GetPlayerCharacter.value(), &params);
        if (!params.ReturnValue) {
            throw std::runtime_error("PlayerCharacter not found");
        }
        return PlayerCharacterWrapper(params.ReturnValue);
    }

}
