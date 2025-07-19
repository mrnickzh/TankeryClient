#pragma once
#include <raylib.h>

namespace PositionConverter {
    inline Vector3 getHalfExtent(Vector3 size) {
        return Vector3(size.x / 2, size.y / 2, size.z / 2);
    };
}
