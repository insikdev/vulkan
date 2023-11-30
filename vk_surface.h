#pragma once

class Window;
class Instance;

class Surface {
public:
    Surface(const Window*, const Instance*);
    ~Surface();

public: // getter
    inline VkSurfaceKHR GetSurface() const { return m_surface; }

private:
    void CreateSurface();

private:
    const Window* p_window;
    const Instance* p_instance;

private:
    VkSurfaceKHR m_surface;
};
