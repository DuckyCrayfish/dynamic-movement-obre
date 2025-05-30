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
