//
//  queue_utils.hpp
//  VulkanTest
//
//  Created by Antonin on 17/06/2022.
//

#ifndef queue_utils_hpp
#define queue_utils_hpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <iostream>
#include <vector>

/**
 * Stores, as optional, queue family indices
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    // For rendered images
    std::optional<uint32_t> present_family;
    
    // TODO: find a better method name...
    bool hasSupport() {
        return graphics_family.has_value() && present_family.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& present_surface);

#endif /* queue_utils_hpp */
