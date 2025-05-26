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

#include "Utils/ModUtils.hpp"


/** @brief This class handles the camera POV locking mechanism. */
class CameraController {
  private:
    bool didLockPOV = false;
    bool previousValuePOV = false;

  public:
    /// Locks the player's POV, preventing it from being changed on-scroll.
    void lockPOV() {
        UObject* playerController = ModUtils::getPlayerController().value();
        bool* bIsPOVChangeLocked = playerController->GetValuePtrByPropertyNameInChain<bool>(STR("bIsPOVChangeLocked"));
        previousValuePOV = *bIsPOVChangeLocked;
        *bIsPOVChangeLocked = true;
        didLockPOV = true;
    }

    /// Resets the POV lock to its previous state, if it was locked by this controller.
    void resetPOVLock() {
        if (!didLockPOV) {
            return;
        }
        didLockPOV = false;
        UObject* playerController = ModUtils::getPlayerController().value();
        bool* bIsPOVChangeLocked = playerController->GetValuePtrByPropertyNameInChain<bool>(STR("bIsPOVChangeLocked"));
        *bIsPOVChangeLocked = previousValuePOV;
    }
};
