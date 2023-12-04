#pragma once

class Query {
public:
    static std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance)
    {
        uint32_t count = 0;

        CHECK_VK(vkEnumeratePhysicalDevices(instance, &count, nullptr));
        assert(count != 0);

        std::vector<VkPhysicalDevice> physicalDevices { count };
        CHECK_VK(vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()));

        return physicalDevices;
    }

    static std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice)
    {
        uint32_t count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
        assert(count != 0);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties { count };
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());

        return queueFamilyProperties;
    }

    static std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        uint32_t count = 0;

        CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr));
        assert(count != 0);

        std::vector<VkSurfaceFormatKHR> surfaceFormats { count };
        CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, surfaceFormats.data()));

        return surfaceFormats;
    }

    static std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        uint32_t count = 0;

        CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr));
        assert(count != 0);

        std::vector<VkPresentModeKHR> presentModes { count };
        CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, presentModes.data()));

        return presentModes;
    }
};