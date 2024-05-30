#ifndef _MJOELNIR_H
#define _MJOELNIR_H

// GLFW headers (includes some vulkan ones)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Vulkan headers
#include <vulkan/vk_enum_string_helper.h>

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <limits.h>

#include <vector>

struct optional_uint32_t {
    bool hasValue;
    uint32_t value;
};

struct QueueFamilyIndices {
    optional_uint32_t graphicsFamily;
    optional_uint32_t presentFamily;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formatCount;
    VkPresentModeKHR* presentModes;
    uint32_t presentModeCount;
};

class Mjoelnir {
private:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    uint32_t swapChainImageCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView *swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer* swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0;

    // There are platform specific surfaces if necessary
    VkSurfaceKHR surface;

    VkShaderModule createShaderModule(uint32_t* code, size_t code_size);
    void initWindow();
    void cleanup();
    bool drawFrame();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    void createInstance();
    bool setupDebugMessenger();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
    bool createLogicalDevice();
    void createSurface();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createGraphicsPipeline();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    bool initVulkan();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t availableFormatsCount);
    VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t availablePresentModesCount);
    void mainLoop();
public:
    void run();
};

#endif
