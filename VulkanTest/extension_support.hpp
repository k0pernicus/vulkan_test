//
//  extension_support.hpp
//  VulkanTest
//
//  Created by Antonin on 17/06/2022.
//

#ifndef extension_support_hpp
#define extension_support_hpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> device_extensions = {
    "VK_KHR_swapchain",
#ifdef __APPLE__
    // **for Apple M1 only**
    // Enable it as, if the VK_KHR_portability_subset
    // extension is included in pProperties of vkEnumerateDeviceExtensionProperties,
    // ppEnabledExtensionNames must include "VK_KHR_portability_subset"
    "VK_KHR_portability_subset"
#endif
};

bool checkValidationLayerSupport();

bool checkDeviceExtensionSupport(VkPhysicalDevice physical_device);

#endif /* extension_support_hpp */
