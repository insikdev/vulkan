#pragma once

#include <fstream>

class Shader {
public:
    static void CreateModule(VkDevice device, const std::string& filename, VkShaderModule* out)
    {
        std::vector<char> binary = ReadFile(filename);

        VkShaderModuleCreateInfo createInfo {};
        {
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = binary.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(binary.data());
        }

        VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, out);
        CHECK_VK(result);
    }

private:
    static std::vector<char> ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        assert(file.is_open());

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
};
