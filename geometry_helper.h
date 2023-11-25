#pragma once
#include "pch.h"

namespace Geometry {
MeshData CreateTriangle()
{
    MeshData out;
    out.vertices = {
        { { 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };
    out.indices = { 0, 1, 2 };
    return out;
}

MeshData CreateRectangle()
{
    MeshData out;
    out.vertices = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } }
    };
    out.indices = { 0, 1, 2, 0, 2, 3 };

    return out;
}
}