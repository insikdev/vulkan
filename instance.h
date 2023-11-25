#pragma once

class Instance {
public:
    Instance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
    ~Instance();
    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&&) = delete;

public: // getter
    inline VkInstance GetInstance() const { return m_instance; }

private:
    void CreateInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

private:
    VkInstance m_instance;
};
