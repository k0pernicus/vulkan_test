//
//  swapchain.hpp
//  VulkanTest
//
//  Created by Antonin on 17/06/2022.
//

#ifndef swapchain_utils_hpp
#define swapchain_utils_hpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include <optional>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

/**
 * Make sure that the SwapChain is adequate for our needs.
 */
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/**
 * Choose the right Surface format, according to your computer / platform.
 */
std::optional<VkSurfaceFormatKHR> chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);

/**
 * Choose the best present mode, according to your computer / platform.
 */
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);

/**
 * Choose the best swap extent mode, according to your computer / platform.
 * The swap extent is the resolution of the swap chain images.
 */
std::optional<VkExtent2D> chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow *window);

#endif /* swapchain_utils_hpp */
