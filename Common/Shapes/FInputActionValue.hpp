#pragma once


struct FInputActionValue {
    float x, y, z;
    int type;
    std::byte padding[16];

    float asFloat() const {
        return y;
    }

    static FInputActionValue Create(bool value) {
        return {0, value ? 1.0f : -1.0f, 0, 0};
    }
};

static_assert(sizeof(FInputActionValue) == 32, "Size mismatch");
