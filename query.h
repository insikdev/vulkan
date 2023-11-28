#pragma once

class Query {
public:
    static std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance)
    {
        uint32_t count = 0;
        std::vector<VkPhysicalDevice> physicalDevices;

        VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);
        CHECK_VK(result);
        assert(count != 0);

        physicalDevices.resize(count);
        result = vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());
        CHECK_VK(result);

        return physicalDevices;
    }

    static std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice)
    {
        uint32_t count = 0;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
        assert(count != 0);

        queueFamilyProperties.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());

        return queueFamilyProperties;
    }

    static std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        VkResult result;

        uint32_t count = 0;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
        CHECK_VK(result);
        assert(count != 0);

        surfaceFormats.resize(count);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, surfaceFormats.data());
        CHECK_VK(result);

        return surfaceFormats;
    }

    static std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        VkResult result;

        uint32_t count;
        std::vector<VkPresentModeKHR> presentModes;

        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
        CHECK_VK(result);
        assert(count != 0);

        presentModes.resize(count);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, presentModes.data());
        CHECK_VK(result);

        return presentModes;
    }
};