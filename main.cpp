#define GLFW_INCLUDE_VULKAN
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

#include "renderer.h"

int main()
{
    Renderer renderer;
    renderer.Run();
    return 0;
}