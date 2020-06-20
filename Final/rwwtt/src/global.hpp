#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <set>
#include <vector>
#include <cstring>
#include <optional>
#include <algorithm>
#include <fstream>
#include <array>

const int max_frames_in_flight = 2;

const uint32_t window_width = 784;
const uint32_t window_height = 441;

const std::vector<const char*> validationLayers{
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

struct Vertex{
    glm::vec2 pos;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions(){
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct UniformBufferObject{
    // ! take care of alignments
    glm::vec2 res;      // window resolution
    float time;         // passed time
    glm::vec2 mouse;    // mouse location
};

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::set<uint32_t> getUniqueSet(){
        if(!isComplete()){
            throw std::runtime_error("** QueueFamilyIndices not complete");
        }
        return std::set<uint32_t>{graphicsFamily.value(), presentFamily.value()};
    }
    bool isComplete(){
        return graphicsFamily.has_value()
            && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

inline bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool allFound = true;
    for (const char *layerName : validationLayers)
    {
        bool found = false;
        for (const auto &layerProperty : availableLayers)
        {
            if (!strcmp(layerName, layerProperty.layerName))
            {
                found = true;
                break;
            }
        }
        allFound = (allFound && found);
        if (!allFound)
        {
            return false;
        }
    }

    return allFound;
}

inline int rateDevice(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeats;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeats);

    if(!deviceFeats.geometryShader){
        return 0;
    }

    int ret = 0;
    ret += deviceProperties.deviceType * 1000;
    ret += deviceProperties.limits.maxImageDimension2D;
    return ret;
}

inline std::vector<char> readFile(const std::string& filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        throw std::runtime_error("** failed to open file");
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

template<class T>
inline T clamp(const T& value, const T& minvalue, const T& maxvalue){
    if(minvalue > maxvalue){
        throw std::runtime_error("** minvalue < maxvalue in global::clamp()");
    }
    return std::max<T>(
        minvalue,
        std::min<T>(
            maxvalue, value
        )
    );
}
