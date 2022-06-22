//
//  queue_utils.cpp
//  VulkanTest
//
//  Created by Antonin on 17/06/2022.
//

#include "queue_utils.hpp"
#include "base.hpp"

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& present_surface) {
    // Get all family queues
    QueueFamilyIndices queue_family_indices {};
    uint32_t family_count {};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, queue_families.data());
    // Find the (first) queue that supports VK_QUEUE_GRAPHICS_BIT & KHR
    // The queue for VK_QUEUE_GRPAHICS_BIT & KHR is not necessarily the same!
    // As an example, the queue 1 can be linked to drawing, and the queue 3 for presentation.
    // TODO: find the physical device that supports BOTH drawing and presentation, to improve perf!
    int i = 0;
    for (const auto& queue_family: queue_families) {
        if (queue_family_indices.hasSupport())
            break;
        if (!queue_family_indices.graphics_family.has_value() && (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            queue_family_indices.graphics_family = i;
        if (!queue_family_indices.present_family.has_value()) {
            VkBool32 present_support = false;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, present_surface, &present_support); present_support)
                queue_family_indices.present_family = i;
        }
        i++;
    }
    if (queue_family_indices.graphics_family.has_value()) {
        Log("-> Found drawing family queue, at index " << queue_family_indices.graphics_family.value());
    }
    if (queue_family_indices.present_family.has_value()) {
        Log("-> Found present family queue, at index " << queue_family_indices.present_family.value());
    }
    return queue_family_indices;
}
