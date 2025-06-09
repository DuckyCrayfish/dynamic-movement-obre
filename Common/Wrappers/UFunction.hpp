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

#include <Unreal/UFunction.hpp>


namespace Wrappers::Detail {
    using UFunction = RC::Unreal::UFunction;
}

namespace Wrappers {

    class UFunctionWrapper {
      protected:
        Detail::UFunction* wrapped;

      public:
        UFunctionWrapper(Detail::UFunction* func) : wrapped(func) {
            if (func == nullptr) {
                throw std::runtime_error("Tried to wrap null UFunction");
            }
        }

        Detail::UFunction* operator->() {
            return wrapped;
        }
        const Detail::UFunction* operator->() const {
            return wrapped;
        }

        Detail::UFunction* value() const {
            return wrapped;
        }

        void* GetParams() {
            return _malloca(wrapped->GetParmsSize());
        }
    };

}
