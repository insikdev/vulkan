#pragma once

class Geometry {
public:
    /*
     * vertex : POSITION + COLOR
     */
    static MeshData CreateTriangle()
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

    /*
     * vertex : POSITION + COLOR
     */
    static MeshData CreateRectangle()
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

    /*
     * vertex : POSITION + COLOR
     */
    static MeshData CreateCube()
    {
        MeshData out;

        out.vertices = {
            // Front face
            { { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

            // Back face
            { { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 1.0f } },

            // Top face
            { { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { 1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

            // Bottom face
            { { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { 1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { 1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },

            // Right face
            { { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f } },
            { { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f } },
            { { 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },

            // Left face
            { { -1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f } }
        };

        // VK_FRONT_FACE_COUNTER_CLOCKWISE
        out.indices = {
            0, 2, 3, 0, 1, 2, // Front face
            4, 6, 7, 4, 5, 6, // Back face
            8, 10, 11, 8, 9, 10, // Top face
            12, 14, 15, 12, 13, 14, // Bottom face
            16, 18, 19, 16, 17, 18, // Right face
            20, 22, 23, 20, 21, 22 // Left face
        };

        return out;
    }
};