//
//  swapchain.cpp
//  VulkanTest
//
//  Created by Antonin on 17/06/2022.
//

#include "swapchain_utils.hpp"

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    SwapChainSupportDetails support_details;
    
    // Query the capabilites of the graphics device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support_details.capabilities);
    
    uint32_t format_count {};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    if (format_count != 0) {
        // Make sure to hold all the available formats
        support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, support_details.formats.data());
    }
    
    uint32_t present_modes_count {};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, nullptr);
    if (present_modes_count != 0) {
        // Make sure to hold all the available present modes
        support_details.present_modes.resize(present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, support_details.present_modes.data());
    }
    return support_details;
}

std::optional<VkSurfaceFormatKHR> chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    // Each VkSurfaceFormatKHR type / struct contains a "format" and a "colorSpace" member.
    // * format: specifies the color channels and type (surface format is the color depth)
    // * colorSpace: SRGB color space is supported or not (SRGB is kind of the standard color space for images)
    for (const auto &available_format: available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return std::nullopt;
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    // Presentation mode represents the actual conditions for showing images to the screen.
    // There are four different options in Vulkan:
    // * VK_PRESENT_MODE_IMMEDIATE_KHR: immediate transfer of images to the screen (can result in tearing)
    // * VK_PRESENT_MODE_FIFO_KHR: most similar to VSync
    // * VK_PRESENT_MODE_FIFO_RELAXED_KHR: like the previous one but could cause tearing too
    // * VK_PRESENT_MODE_MAILBOX_KHR: (triple buffering) similar to VK_PRESENT_MODE_FIFO_KHR but no blocking queue: avoid tearing but fewer latency issues than standard VSync.
    // Best mode here is VK_PRESENT_MODE_MAILBOX_KHR but it is not available everywhere like VK_PRESENT_MODE_FIFO_KHR.
    // Also, VK_PRESENT_MODE_MAILBOX_KHR can introduces extensive energy usage.
    // As a remember, only VK_PRESENT_MODE_FIFO_KHR is available, as a mode, in every platform.
    for (const auto &available_present_mode: available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }
    // Use VK_PRESENT_MODE_FIFO_KHR as a "back-end" (or default) value.
    return VK_PRESENT_MODE_FIFO_KHR;
}

std::optional<VkExtent2D> chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow *window) {
    if (window == nullptr) {
        return std::nullopt;
    }
    // The range of the possible resolutions is defined in
    // the VkSurfaceCapabilitiesKHR structure, in the currentExtent member.
    // Also, both height / width should **not** be 0.
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    int width, height {};
    glfwGetFramebufferSize(window, &width, &height);
    VkExtent2D current_extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    // Bound the final values of height and width
    current_extent.height = std::clamp(current_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    current_extent.width = std::clamp(current_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    
    return current_extent;
}
