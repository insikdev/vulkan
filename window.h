#pragma once

class App;

class Window {
public:
    Window(const char* title, App*);
    ~Window();
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

public:
    bool ShouldClose();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

public: // getter setter
    inline GLFWwindow* GetWindow() const { return m_window; }
    inline void SetWidth(int width) { m_width = width; }
    inline void SetHeight(int height) { m_height = height; }

private:
    void CreateGLFWWindow(const char*);

private:
    App* p_app;

private:
    int m_width = 800;
    int m_height = 800;
    GLFWwindow* m_window;
};
