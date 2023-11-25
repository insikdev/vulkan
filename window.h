#pragma once

class Window {
public:
    Window(const char* title);
    ~Window();
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

public:
    bool ShouldClose();

public: // getter
    inline GLFWwindow* GetWindow() const { return m_window; }

private:
    void CreateGLFWWindow(const char*);

private:
    int m_width = 800;
    int m_height = 800;
    GLFWwindow* m_window;
};
