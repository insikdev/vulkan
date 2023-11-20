#pragma once

#include <GLFW/glfw3.h>
#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();

public:
    void Run();

private:
    void InitWindow();

private:
    GLFWwindow* m_window;
};
