#include "mjoelnir.hpp"
#include <iostream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifndef NDEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};

const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_portability_subset",
};

uint32_t uint32_t_clamp(uint32_t value, uint32_t min, uint32_t max) {
    if (value <= min) {
        return min;
    }

    if (value >= max) {
        return max;
    }

    return value;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    /*
        The first parameter specifies the severity of the message, which is one of the following flags:
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes
     */
    /*
        The messageType parameter can have the following values:
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan
     */
    /*
        The pCallbackData parameter refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing the details of the message itself, with the most important members being:
            pMessage: The debug message as a null-terminated string
            pObjects: Array of Vulkan object handles related to the message
            objectCount: Number of objects in array
     */

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        fprintf(stderr, "\033[1m\033[38;5;9m");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "\033[1m\033[38;5;11m");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        fprintf(stderr, "\033[2m\033[38;5;45m");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        fprintf(stderr, "\033[2m");
    }

    fprintf(stderr, "Debug message: %s\n", pCallbackData->pMessage);

    fprintf(stderr, "\033[0m");

    return VK_FALSE;
}

uint32_t* readFileBinary(const char* filename, size_t* file_size) {
    FILE* file_ptr = fopen(filename, "rb");
    if (!file_ptr) {
        fprintf(stderr, "Unable to read file: %s\n", filename);
        return NULL;
    }

    fseek(file_ptr, 0L, SEEK_END);
    *file_size = ftell(file_ptr);

    // fseek(file_ptr, 0L, SEEK_SET);
    rewind(file_ptr);

    uint32_t* file_buffer = (uint32_t*)malloc(*file_size * sizeof(uint32_t));
    fread(file_buffer, sizeof(uint32_t), *file_size/sizeof(uint32_t), file_ptr);

    fclose(file_ptr);

    return file_buffer;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount*sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    printf("\033[2mAvailable layers:\n");
    for (int i = 0; i < layerCount; i++) {
        printf("\t%s\n", availableLayers[i].layerName);
    }
    printf("\033[0m");

    for (int i = 0; i < sizeof(validationLayers)/sizeof(const char*); i++) {
        bool found = false;

        for (int j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "Missing required extension %s\n", validationLayers[i]);
            return false;
        }
    }

    return true;
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

bool isDeviceComplete(QueueFamilyIndices *indices) {
    return indices->graphicsFamily.hasValue && indices->presentFamily.hasValue;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t availableExtensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount, NULL);

    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(availableExtensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount, availableExtensions);

    uint32_t requiredExtensionCount = (uint32_t)(sizeof(deviceExtensions)/sizeof(const char*));

    uint32_t foundExtensionCount = 0;
    for (int i = 0; i < requiredExtensionCount; i++) {
        bool found = false;
        for (int j = 0; j < availableExtensionCount; j++) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (found) {
            foundExtensionCount++;
        } else {
            fprintf(stderr, "Missing extension support for: %s\n", deviceExtensions[i]);
        }
    }

    return foundExtensionCount == requiredExtensionCount;
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT* createInfo
) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    createInfo->pUserData = NULL;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, createInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

const char** getRequiredExtensions(uint32_t* requiredExtensionCount) {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    // Does this needs to be freed?
    VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("\033[2mAvailable extensions:\n");
    for (int i = 0; i < extensionCount; i++) {
        printf("\t%s\n", extensions[i].extensionName);
    }
    printf("\033[0m");

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (int i = 0; i < glfwExtensionCount; i++) {
        bool found = false;

        for (int j = 0; j < extensionCount; j++) {
            if (strcmp(glfwExtensions[i], extensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "Missing required extension %s\n", glfwExtensions[i]);
            return NULL;
        }
    }

    const char* additionalRequiredExtensions[] = {
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    };
    uint32_t additionalRequiredExtensionCount = (uint32_t)(sizeof(additionalRequiredExtensions)/sizeof(const char*));

    if (enableValidationLayers) {
        additionalRequiredExtensionCount++;
    }

    // todo: rewrite this login to instead use 1 realloc call to have everything in 1 nice array instead of 2 with one on the side.
    const char** requiredExtensions = (const char**)malloc((glfwExtensionCount + additionalRequiredExtensionCount) * sizeof(const char*));
    for (int i = 0; i < glfwExtensionCount + 1; i++) {
        // Does this needs to be freed?
        requiredExtensions[i] = (const char*)malloc(strlen(glfwExtensions[i]+1));
        requiredExtensions[i] = glfwExtensions[i];
    }

    // Does this needs to be freed?
    requiredExtensions[glfwExtensionCount] = (const char*)malloc(strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)+1);
    requiredExtensions[glfwExtensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    // Does this needs to be freed?
    requiredExtensions[glfwExtensionCount+1] = (const char*)malloc(strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)+1);
    requiredExtensions[glfwExtensionCount+1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    requiredExtensions[glfwExtensionCount+2] = (const char*)malloc(strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)+1);
    requiredExtensions[glfwExtensionCount+2] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;

    *requiredExtensionCount = glfwExtensionCount + additionalRequiredExtensionCount;

    printf("\033[2mmRequired extensions:\n");
    for (int i = 0; i < *requiredExtensionCount; i++) {
        printf("\t%s\n", requiredExtensions[i]);
    }
    printf("\033[0m");

    return requiredExtensions;
}

VkShaderModule Mjoelnir::createShaderModule(uint32_t* code, size_t code_size) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code_size;

  createInfo.pCode = code;

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
      fprintf(stderr, "Unable to create shader module\n");
      return NULL;
  }

  return shaderModule;
}

void Mjoelnir::initWindow() {
  #ifndef NDEBUG
      printf("\033[38;5;9m[[ DEBUG ]]\033[0m\n");
  #else
      printf("\033[38;5;9m[[ RELEASE ]]\033[0m\n");
  #endif
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Mjoelnir", NULL, NULL);
}

void Mjoelnir::cleanup() {
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
      vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
      vkDestroyFence(device, inFlightFences[i], NULL);
  }

  vkDestroyCommandPool(device, commandPool, NULL);

  for (uint32_t i = 0; i < swapChainImageCount; i++) {
      vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
  }

  vkDestroyPipeline(device, graphicsPipeline, NULL);
  vkDestroyPipelineLayout(device, pipelineLayout, NULL);
  vkDestroyRenderPass(device, renderPass, NULL);

  for (uint32_t i = 0; i < swapChainImageCount; i++) {
      vkDestroyImageView(device, swapChainImageViews[i], NULL);
  }

  vkDestroySwapchainKHR(device, swapChain, NULL);
  vkDestroyDevice(device, NULL);

  if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
  }

  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);

  glfwDestroyWindow(window);
  glfwTerminate();
}

bool Mjoelnir::drawFrame() {
  // At a high level, rendering a frame in Vulkan consists of a common set of steps:
  // Wait for the previous frame to finish
  // Acquire an image from the swap chain
  // Record a command buffer which draws the scene onto that image
  // Submit the recorded command buffer
  // Present the swap chain image

  vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
  vkResetFences(device, 1, &inFlightFences[currentFrame]);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  vkResetCommandBuffer(commandBuffers[currentFrame], 0);

  recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
      fprintf(stderr, "Failed to submit draw command buffer\n");
      return false;
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  presentInfo.pResults = NULL; // Optional

  vkQueuePresentKHR(presentQueue, &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return true;
}

SwapChainSupportDetails Mjoelnir::querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);

  if (details.formatCount > 0) {
      details.formats = (VkSurfaceFormatKHR*)malloc(details.formatCount * sizeof(VkSurfaceFormatKHR));
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, details.formats);
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);

  if (details.presentModeCount > 0) {
      details.presentModes = (VkPresentModeKHR*)malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, details.presentModes);
  }

  return details;
}

bool Mjoelnir::createInstance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
      return false;
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Mjoelnir Sandbox";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Mjoelnir";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

  uint32_t extensionCount = 0;
  const char** extensions = getRequiredExtensions(&extensionCount);

  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
  if (enableValidationLayers) {
      createInfo.enabledLayerCount = (uint32_t)(sizeof(validationLayers)/sizeof(const char*));
      createInfo.ppEnabledLayerNames = validationLayers;

      populateDebugMessengerCreateInfo(&debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  } else {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = NULL;
  }

  VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
  if (result != VK_SUCCESS) {
      fprintf(stderr, "Unable to create instance (%s)\n", string_VkResult(result));
      return false;
  }

  return true;
}

bool Mjoelnir::setupDebugMessenger() {
  if (!enableValidationLayers) {
      return true;
  }

  VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
  populateDebugMessengerCreateInfo(&createInfo);

  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
      fprintf(stderr, "Unable to create debug messenger\n");
      return false;
  }

  return true;
}

QueueFamilyIndices Mjoelnir::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

  // The VkQueueFamilyProperties struct contains some details about the queue family,
  // including the type of operations that are supported and the number of queues that can be created based on that family.
  VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

  for (int i = 0; i < queueFamilyCount; i++) {
      // todo: For improved performance, prioritize devices that support both graphics and present.
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          indices.graphicsFamily.hasValue = true;
          indices.graphicsFamily.value = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (presentSupport) {
          indices.presentFamily.hasValue = true;
          indices.presentFamily.value = i;
      }

      if (isDeviceComplete(&indices)) {
          break;
      }
  }

  return indices;
}

VkExtent2D Mjoelnir::chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities) {
  if (capabilities->currentExtent.width != UINT32_MAX) {
      return capabilities->currentExtent;
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  VkExtent2D actualExtent;

  actualExtent.width = uint32_t_clamp((uint32_t)width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
  actualExtent.height = uint32_t_clamp((uint32_t)height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

  return actualExtent;
}

bool Mjoelnir::isDeviceSuitable(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  // todo: add some sort of GPU priority

  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = swapChainSupport.formatCount > 0 && swapChainSupport.presentModeCount > 0;
  }

  return isDeviceComplete(&indices) && extensionsSupported && swapChainAdequate;
}

bool Mjoelnir::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

  if (deviceCount == 0) {
      return false;
  }

  VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  for (int i = 0; i < deviceCount; i++) {
      if (isDeviceSuitable(devices[i])) {
          physicalDevice = devices[i];
          return true;
      }
  }

  // if (physicalDevice == VK_NULL_HANDLE) {
  //     return false;
  // }

  return false;
}

bool Mjoelnir::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  uint32_t queueFamilies[] = {indices.graphicsFamily.value, indices.presentFamily.value};
  uint32_t queueFamilyCount = sizeof(queueFamilies) / sizeof(uint32_t);
  uint32_t uniqueQueueFamilyCount = 0;
  uint32_t* uniqueQueueFamilies = (uint32_t*)malloc(0);

  for (int i = 0; i < queueFamilyCount; i++) {
      bool in = false;

      for (int j = 0; j < uniqueQueueFamilyCount; j++) {
          if (queueFamilies[i] == uniqueQueueFamilies[j]) {
              in = true;
              break;
          }
      }

      if (!in) {
          uniqueQueueFamilyCount++;
          uniqueQueueFamilies = (uint32_t*)realloc(uniqueQueueFamilies, uniqueQueueFamilyCount);
          uniqueQueueFamilies[uniqueQueueFamilyCount-1] = queueFamilies[i];
      }
  }

  VkDeviceQueueCreateInfo* queueCreateInfos = (VkDeviceQueueCreateInfo*)malloc(uniqueQueueFamilyCount * sizeof(VkDeviceQueueCreateInfo));
  for (int i = 0; i < uniqueQueueFamilyCount; i++) {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
      queueCreateInfo.queueCount = 1;

      // queue priorities between 0.0 and 1.0
      float queuePriority = 1.0f;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos[i] = queueCreateInfo;
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.queueCreateInfoCount = uniqueQueueFamilyCount;

  createInfo.pEnabledFeatures = &deviceFeatures;

  uint32_t deviceExtensionCount = sizeof(deviceExtensions)/sizeof(const char*);

  createInfo.enabledExtensionCount = deviceExtensionCount;
  createInfo.ppEnabledExtensionNames = deviceExtensions;

  if (enableValidationLayers) {
      createInfo.enabledLayerCount = (sizeof(validationLayers)/sizeof(const char*));
      createInfo.ppEnabledLayerNames = validationLayers;
  } else {
      createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
      fprintf(stderr, "Failed to create logical device\n");
      return false;
  }

  vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value, 0, &presentQueue);

  return true;
}

bool Mjoelnir::createSurface() {
  if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
      fprintf(stderr, "Failed to create window surface\n");
      return false;
  }

  return true;
}

bool Mjoelnir::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
  VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  // swapChainSupport.capabilities.maxImageCount == 0 means there is no maximum.
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  // The imageArrayLayers specifies the amount of layers each image consists of
  // This is always 1 unless you are developing a stereoscopic 3D application
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value, indices.presentFamily.value};

  if (indices.graphicsFamily.value != indices.presentFamily.value) {
      // Images can be used across multiple queue families without explicit ownership transfers
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
      // An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family
      // This option offers the best performance
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = NULL;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  // The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system
  // You’ll almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS) {
      fprintf(stderr, "Failed to create swap chain\n");
      return false;
  }

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;

  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
  swapChainImages = (VkImage*)malloc(imageCount * sizeof(VkImage));
  swapChainImageCount = imageCount;
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

  return true;
}

bool Mjoelnir::createImageViews() {
  swapChainImageViews = (VkImageView*)malloc(swapChainImageCount * sizeof(VkImageView));

  for (uint32_t i = 0; i < swapChainImageCount; i++) {
      VkImageViewCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];
      
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;
      
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      // If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers.
      // You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers.
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
          fprintf(stderr, "Failed to create image view\n");
          return false;
      }
  }

  return true;
}

bool Mjoelnir::createRenderPass() {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = swapChainImageFormat;
  // No multisampling yet, stick to 1 bit.
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

  // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
  // VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
  // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don’t care about them
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
  // VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  // Not using stencil buffers.
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
  subpass.pColorAttachments = &colorAttachmentRef;
  // The following other types of attachments can be referenced by a subpass:
  //     pInputAttachments: Attachments that are read from a shader
  //     pResolveAttachments: Attachments used for multisampling color attachments
  //     pDepthStencilAttachment: Attachment for depth and stencil data
  //     pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  // The VkAttachmentReference objects reference attachments using the indices of this array.
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS) {
      fprintf(stderr, "Failed to create render pass\n");
      return false;
  }

  return true;
}

bool Mjoelnir::createGraphicsPipeline() {
  size_t vert_shader_size;
  size_t frag_shader_size;
  // todo: Need a good way to handle these paths without being so specific.
  uint32_t* vert_shader_code = readFileBinary("/Users/singmyr/dev/mjoelnir/Mjoelnir/src/shaders/shader_vert.spv", &vert_shader_size);
  uint32_t* frag_shader_code = readFileBinary("/Users/singmyr/dev/mjoelnir/Mjoelnir/src/shaders/shader_frag.spv", &frag_shader_size);

  VkShaderModule vert_shader_module = createShaderModule(vert_shader_code, vert_shader_size);
  VkShaderModule frag_shader_module = createShaderModule(frag_shader_code, frag_shader_size);

  // todo: Necessary?
  // free(vert_shader_code);
  // free(frag_shader_code);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vert_shader_module;
  vertShaderStageInfo.pName = "main";
  vertShaderStageInfo.pSpecializationInfo = NULL;
  /*
      pSpecializationInfo

      It allows you to specify values for shader constants.
      You can use a single shader module where its behavior can be configured at pipeline creation by specifying different values for the constants used in it.
      This is more efficient than configuring the shader using variables at render time, because the compiler can do optimizations like eliminating if statements that depend on these values.
      If you don’t have any constants like that, then you can set the member to nullptr, which our struct initialization does automatically.
  */
  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = frag_shader_module;
  fragShaderStageInfo.pName = "main";
  fragShaderStageInfo.pSpecializationInfo = NULL;

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = NULL;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = NULL;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
  // VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
  // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
  // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
  // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  // If you set the primitiveRestartEnable member to VK_TRUE, then it’s possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  VkOffset2D scissorOffset = {};
  scissorOffset.x = 0;
  scissorOffset.y = 0;
  scissor.offset = scissorOffset;
  scissor.extent = swapChainExtent;

  VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
  };
  // VkDynamicState* dynamicStates = malloc(2 * sizeof(VkDynamicState));
  // dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
  // dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.
  // This is useful in some special cases like shadow maps.
  // Using this requires enabling a GPU feature.
  rasterizer.depthClampEnable = VK_FALSE;
  // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the rasterizer stage.
  // This basically disables any output to the framebuffer.
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  // The polygonMode determines how fragments are generated for geometry.
  // The following modes are available:
  //   VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
  //   VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
  //   VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
  // Using any mode other than fill requires enabling a GPU feature.
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  // The lineWidth member is straightforward, it describes the thickness of lines in terms of number of fragments.
  // The maximum line width that is supported depends on the hardware and any line thicker than 1.0f requires you to enable the wideLines GPU feature.
  rasterizer.lineWidth = 1.0f;
  // The cullMode variable determines the type of face culling to use.
  // You can disable culling, cull the front faces, cull the back faces or both.
  // The frontFace variable specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise.
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  // Enabling multisampling requires enabling a GPU feature.
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = NULL; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  // Psuedo-code of how the color blending will be performed:
  // if (blendEnable) {
  //     finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
  //     finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
  // } else {
  //     finalColor = newColor;
  // }
  // finalColor = finalColor & colorWriteMask;
  
  // If blendEnable is set to VK_FALSE, then the new color from the fragment shader is passed through unmodified.
  // Otherwise, the two mixing operations are performed to compute a new color.
  // The resulting color is AND’d with the colorWriteMask to determine which channels are actually passed through.
  
  // The most common way to use color blending is to implement alpha blending, where we want the new color to be blended with the old color based on its opacity.
  // The finalColor should then be computed as follows:

  // finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
  // finalColor.a = newAlpha.a;
  // This can be accomplished with the following parameters:

  // colorBlendAttachment.blendEnable = VK_TRUE;
  // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = NULL;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = NULL;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
      fprintf(stderr, "Unable to create pipeline layout\n");
      return false;
  }

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;

  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = NULL; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;

  pipelineInfo.layout = pipelineLayout;

  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS) {
      fprintf(stderr, "Unable to create graphics pipeline\n");
      return false;
  }
  
  vkDestroyShaderModule(device, vert_shader_module, NULL);
  vkDestroyShaderModule(device, frag_shader_module, NULL);

  return true;
}

bool Mjoelnir::createFramebuffers() {
  swapChainFramebuffers = (VkFramebuffer*)malloc(swapChainImageCount * sizeof(VkFramebuffer));

  for (uint32_t i = 0; i < swapChainImageCount; i++) {
      VkImageView attachments[] = {
          swapChainImageViews[i]
      };

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]) != VK_SUCCESS) {
          fprintf(stderr, "Unable to create framebuffer\n");
          return false;
      }
  }

  return true;
}

bool Mjoelnir::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value;
  
  if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) != VK_SUCCESS) {
      fprintf(stderr, "Unable to create command pool\n");
      return false;
  }

  return true;
}

bool Mjoelnir::createCommandBuffers() {
  commandBuffers = (VkCommandBuffer*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
  // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) != VK_SUCCESS) {
      fprintf(stderr, "Unable to create command buffer\n");
      return false;
  }

  return true;
}

bool Mjoelnir::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // The flags parameter specifies how we’re going to use the command buffer. The following values are available:
  //     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
  //     VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
  //     VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = NULL; // Optional

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      fprintf(stderr, "Unable to begin recording command buffer\n");
      return false;
  }

  VkRenderPassBeginInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
  VkOffset2D renderAreaOffset = {};
  renderAreaOffset.x = 0;
  renderAreaOffset.y = 0;
  renderPassInfo.renderArea.offset = renderAreaOffset;
  renderPassInfo.renderArea.extent = swapChainExtent;

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  // The render pass can now begin.
  // All of the functions that record commands can be recognized by their vkCmd prefix.
  // They all return void, so there will be no error handling until we’ve finished recording.

  // The first parameter for every command is always the command buffer to record the command to.
  // The second parameter specifies the details of the render pass we’ve just provided.
  // The final parameter controls how the drawing commands within the render pass will be provided.
  // It can have one of two values:
  //     VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
  //     VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor = {};
  VkOffset2D scissorOffset = {};
  scissorOffset.x = 0;
  scissorOffset.y = 0;
  scissor.offset = scissorOffset;
  scissor.extent = swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // vertexCount: Even though we don’t have a vertex buffer, we technically still have 3 vertices to draw.
  // instanceCount: Used for instanced rendering, use 1 if you’re not doing that.
  // firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
  // firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      fprintf(stderr, "Unable to record command buffer\n");
      return false;
  }

  return true;
}

bool Mjoelnir::createSyncObjects() {
  imageAvailableSemaphores = (VkSemaphore*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
  renderFinishedSemaphores = (VkSemaphore*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));

  inFlightFences = (VkFence*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  // Makes this fence start signaled.
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
          vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
          vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) != VK_SUCCESS
      ) {
          fprintf(stderr, "Failed to create synchronization objects\n");
          return false;
      }
  }

  return true;
}

bool Mjoelnir::initVulkan() {
  if (!createInstance()) {
      fprintf(stderr, "Unable to create instance\n");
      return false;
  }

  setupDebugMessenger();

  if (!createSurface()) {
      fprintf(stderr, "Failed to create window surface\n");
      return false;
  }

  if (!pickPhysicalDevice()) {
      fprintf(stderr, "Failed to find GPUs with Vulkan support\n");
      return false;
  }

  if (!createLogicalDevice()) {
      fprintf(stderr, "Failed to create logical device\n");
      return false;
  }

  if (!createSwapChain()) {
      fprintf(stderr, "Failed to create swap chain\n");
      return false;
  }

  if (!createImageViews()) {
      fprintf(stderr, "Failed to create image views\n");
      return false;
  }

  if (!createRenderPass()) {
      fprintf(stderr, "Failed to create render pass\n");
      return false;
  }

  if (!createGraphicsPipeline()) {
      fprintf(stderr, "Failed to create graphics pipeline\n");
      return false;
  }

  if (!createFramebuffers()) {
      fprintf(stderr, "Failed to create frame buffers\n");
      return false;
  }

  if (!createCommandPool()) {
      fprintf(stderr, "Failed to create command pool\n");
      return false;
  }

  if (!createCommandBuffers()) {
      fprintf(stderr, "Failed to create command buffer\n");
      return false;
  }

  if (!createSyncObjects()) {
      fprintf(stderr, "Failed to create synchronization objects\n");
      return false;
  }

  return true;
}

VkSurfaceFormatKHR Mjoelnir::chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t availableFormatsCount) {
  for (int i = 0; i < availableFormatsCount; i++) {
      if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
          return availableFormats[i];
      }
  }

  // Here we are assuming that we already checked that we have a non-zero amount of available formats
  // todo: could rank the available formats in some way since we didn't find an "optimal" one.
  return availableFormats[0];
}

VkPresentModeKHR Mjoelnir::chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t availablePresentModesCount) {
  // VK_PRESENT_MODE_FIFO_KHR preferred on mobile devices?
  // VK_PRESENT_MODE_MAILBOX_KHR = triple buffering
  // VK_PRESENT_MODE_FIFO_KHR = always available

  for (int i = 0; i < availablePresentModesCount; i++) {
      if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
          return availablePresentModes[i];
      }
  }

  return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

void Mjoelnir::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      if (!drawFrame()) {
          break;
      }
  }

  vkDeviceWaitIdle(device);
}

void Mjoelnir::run() {
  initWindow();

  if (!initVulkan()) {
      return;
  }

  mainLoop();

  cleanup();
}
