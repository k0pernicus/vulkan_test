//
//  main.cpp
//  VulkanTest
//
//  Created by Antonin on 12/06/2022.
//

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <filesystem>
#ifdef _WIN32
#include <assert.h>
#endif

#include "base.hpp"
#include "extension_support.hpp"
#include "queue_utils.hpp"
#include "swapchain_utils.hpp"
#include "shader_support.hpp"

#ifdef DEBUG
constexpr const bool enable_validation_layers = true;
#else
constexpr const bool enable_validation_layers = false;
#endif

#define _ENABLE_COMPATIBILITY_WITH_OLDER_VK_IMPL 1

constexpr const char* APPLICATION_TITLE = "Vulkan window";
constexpr uint32_t const HEIGHT = 580;
constexpr uint32_t const WIDTH = 700;

constexpr uint8_t const APP_MAJOR_VERSION = 1;
constexpr uint8_t const APP_MINOR_VERSION = 0;
constexpr uint8_t const APP_PATCH_VERSION = 0;
constexpr uint32_t const APP_VERSION VK_MAKE_VERSION(
    APP_MAJOR_VERSION,
    APP_MINOR_VERSION,
    APP_PATCH_VERSION
);

constexpr const char* ENGINE_NAME = "Frame Engine";
constexpr uint8_t const ENGINE_MAJOR_VERSION = 0;
constexpr uint8_t const ENGINE_MINOR_VERSION = 1;
constexpr uint8_t const ENGINE_PATCH_VERSION = 0;
constexpr uint32_t const ENGINE_VERSION VK_MAKE_VERSION(
    ENGINE_MAJOR_VERSION,
    ENGINE_MINOR_VERSION,
    ENGINE_PATCH_VERSION
);

class TriangleApplication {
    
public:
    void run() {
        /// Initializes the GLFW library and creates a window with a proper configuration
        initWindow();
        /// Initializes the Vulkan library, and link to the app
        initSystem();
        /// The main loop to render the app
        loop();
        ///  Destroy all created instances
        clean();
    }
    
    /// The constructor of the Triangle app / example
    TriangleApplication() {}
    
    ~TriangleApplication() {
        m_app_window = nullptr;
        m_vk_instance = NULL;
        m_graphics_device = VK_NULL_HANDLE;
        m_logical_graphics_device = NULL;
        m_graphics_queue = NULL;
        m_present_queue = NULL;
        m_surface = NULL;
        m_swap_chain = NULL;
        m_swap_chain_images.clear();
        m_swap_chain_image_views.clear();
    }
    
private:
    // GLFW / GLW related
    GLFWwindow *m_app_window = nullptr;
    // Vulkan related
    VkInstance m_vk_instance {};
    // The graphics device
    VkPhysicalDevice m_graphics_device = VK_NULL_HANDLE;
    // Logical graphics device to communicate with
    VkDevice m_logical_graphics_device = NULL;
    // Stores an handle to the drawing / graphics queue,
    // and the presentation one
    // Device queues are automatically cleaned up
    // so no need to handle / free them in the clean function
    VkQueue m_graphics_queue, m_present_queue = NULL;
    // Abstract type of surface to send rendered images
    VkSurfaceKHR m_surface = NULL;
    // The swap chain
    VkSwapchainKHR m_swap_chain = NULL;
    // Set of images to be drawn and present
    // to the window
    std::vector<VkImage> m_swap_chain_images;
    // Views to draw images in the swap chain
    std::vector<VkImageView> m_swap_chain_image_views;
    // The retrieved and stored image format for our swap chain
    VkSurfaceFormatKHR m_swap_chain_surface_format;
    // The retrieved and stored extent of our swap chain
    VkExtent2D m_swap_chain_extent;
    // The graphics pipeline layout, for
    // uniform values
    VkPipelineLayout m_pipeline_layout;
    // Render pass process
    VkRenderPass m_render_pass;
    // The graphics pipeline
    VkPipeline m_graphics_pipeline;
    // Attachments specified during render pass creation
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    // Create the command pool to create
    // command buffers
    VkCommandPool m_command_pool;
    VkCommandBuffer m_command_buffer;
    // Signal that an image has been acquired from the swapchain
    // and is ready for rendering
    VkSemaphore m_image_avail_semaphore;
    // Signal that rendering has been finished and
    // presentation can happen
    VkSemaphore m_render_finished_semaphore;
    // Make sure only one frame is rendering at a time
    VkFence m_in_flight_fence;
    
    VkApplicationInfo _createAppInfo() {
        // Create a Vulkan app info
        VkApplicationInfo app_info {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = APPLICATION_TITLE;
        app_info.applicationVersion = APP_VERSION;
        app_info.pEngineName = ENGINE_NAME;
        app_info.engineVersion = ENGINE_VERSION;
        // Change to VK_API_VERSION_1_0 for retrocompatibility?
        app_info.apiVersion = VK_API_VERSION_1_3;
        return app_info;
    }
    
    VkInstanceCreateInfo _createInstanceCreateInfo(const VkApplicationInfo &app_info) {
        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;
        return instance_create_info;
    }
    
    void _initVulkan() {
        Log("############################");
        Log("Initializing Vulkan instance ...");
        Log("############################");
        if (enable_validation_layers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
            return;
        }
        const VkApplicationInfo app_info = _createAppInfo();
        VkInstanceCreateInfo instance_create_info = _createInstanceCreateInfo(app_info);
        // As Vulkan is platform agnostic API, and we use GLFW for window management, check
        // the available extensions
        uint32_t glfw_extensions_count {};
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
        instance_create_info.enabledExtensionCount = glfw_extensions_count;
        instance_create_info.ppEnabledExtensionNames = glfw_extensions;
        
        uint32_t extension_count {};
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());
#ifdef DEBUG
        std::cout << "Available instance extensions:\n";
        for (const auto &extension: available_extensions) {
            Log("\t " << extension.extensionName);
        }
#endif
        
        if (enable_validation_layers) {
            instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            instance_create_info.ppEnabledLayerNames = validation_layers.data();
        } else
            instance_create_info.enabledLayerCount = 0;
        
        const auto res = vkCreateInstance(&instance_create_info, nullptr, &m_vk_instance);
        
        if (res == VK_SUCCESS) return; // Only success case
        
        switch (res) {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                throw std::runtime_error("failed to create Vulkan instance: out of host memory");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                throw std::runtime_error("failed to create Vulkan instance: out of device memory");
                break;
            case VK_ERROR_LAYER_NOT_PRESENT:
                throw std::runtime_error("failed to create Vulkan instance: layer not present");
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                throw std::runtime_error("failed to create Vulkan instance: extension not present");
                break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                throw std::runtime_error("failed to create Vulkan instance: incompatible driver");
                break;
            default:
                throw std::runtime_error("failed to create Vulkan instance");
                break;
        }
    }
    
    void _createSurface() {
        Log("##########################");
        Log("Creating window surface...");
        Log("##########################");
        if (const auto res = glfwCreateWindowSurface(m_vk_instance, m_app_window, nullptr, &m_surface); res != VK_SUCCESS) {
            switch (res) {
                case VK_ERROR_INITIALIZATION_FAILED:
                    throw std::runtime_error("cannot create a window surface: GLFW is not initialized");
                    break;
                case VK_ERROR_EXTENSION_NOT_PRESENT:
                    throw std::runtime_error("cannot create a window surface: GLFW API is unavailable");
                    break;
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                    throw std::runtime_error("cannot create a window surface: GLFW platform error");
                    break;
                default:
                    throw std::runtime_error("cannot create a window surface: unknown error");
                    break;
            }
            return;
        }
        
    }
    
    void _pickGraphicsDevice() {
        Log("#############################");
        Log("Choosing a physical device...");
        Log("#############################");
        uint32_t device_count {};
        vkEnumeratePhysicalDevices(m_vk_instance, &device_count, nullptr);
        if (device_count == 0) {
            throw std::runtime_error("no graphics device (GPU) found with Vulkan support");
            return;
        }
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(m_vk_instance, &device_count, devices.data());
#ifdef DEBUG
        std::cout << "Available devices:\n";
        for (const VkPhysicalDevice &device: devices) {
            VkPhysicalDeviceProperties device_properties {};
            vkGetPhysicalDeviceProperties(device, &device_properties);
            std::cout << "* " << device_properties.deviceName << " ("
                      << "deviceType: " << device_properties.deviceType << ", "
                      << "apiVersion: " << device_properties.apiVersion << ", "
                      << "deviceID: " << device_properties.deviceID << ", "
                      << "vendorID: " << device_properties.vendorID << ")\n";
        }
#endif
        // TODO: a good algorithm to pick the "best" device
        // Compute a score to favor a dedicated graphics card (higher score),
        // but fall back to an iGPU if no dGPU has been found
        VkPhysicalDeviceFeatures device_features {};
        vkGetPhysicalDeviceFeatures(devices[0], &device_features);
#ifndef __APPLE__
        // Disable for now as m1 macs do not support geometry shader
        // with Apple Metal (< 3.x)
        if (!device_features.geometryShader) {
            // A Vulkan app does not work without geometryShader feature
            throw std::runtime_error("failed to find a graphice device (GPU) with geometryShader support");
            return;
        }
#endif
        m_graphics_device = devices[0];
        Log("-> Checking the graphics device... ");
        if (m_graphics_device == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a graphice device (GPU) with Vulkan support");
            return;
        }
        Log("-> Checking the device extension support... ");
        if (!checkDeviceExtensionSupport(m_graphics_device)) {
            std::cout << "failed!" << std::endl;
            throw std::runtime_error("device extensions have not been found");
            return;
        }
        Log("-> Checking the swapchain support... ");
        // Make sure the SwapChain is adequate for our needs
        SwapChainSupportDetails swapchain_support = querySwapChainSupport(m_graphics_device, m_surface);
        if (swapchain_support.formats.empty() || swapchain_support.present_modes.empty()) {
            throw std::runtime_error("swapchain support is incorrect on your device");
            return;
        }
    }
    
    /**
     * Based on the graphics device (or driver), initializes the logical device, and
     * the Queue Create Info information.
     */
    void _initLogicalGraphicsDevice() {
        Log("##########################");
        Log("Init the logical device...");
        Log("##########################");
        if (m_graphics_device == NULL) return;
        QueueFamilyIndices queue_family_indices = findQueueFamilies(m_graphics_device, m_surface);
        
        // Only 2 queues available in QueueFamilyIndices
        // TODO: find a better way to handle the number of queues
        VkDeviceQueueCreateInfo queue_create_infos[2] {};
        std::set<uint32_t> queue_families = {queue_family_indices.graphics_family.value(), queue_family_indices.present_family.value()};
        
        float priority = 1.0f;
        uint32_t i = 0;
        for (uint32_t queue_family: queue_families) {
            VkDeviceQueueCreateInfo queue_create_info {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            // As we can create all the command buffers on multiple threads, and
            // then submit them all at once on the main thread (with a single
            // low-overhead call), we only have to create one queue
            queue_create_info.queueCount = 1;
            // High priority as we have only one queue
            queue_create_info.pQueuePriorities = &priority;
            queue_create_infos[i] = queue_create_info;
            ++i;
        }
        
        // Enable all device features for now
        // TODO: change to specific features
        VkPhysicalDeviceFeatures device_features {};
        VkDeviceCreateInfo device_create_info {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_infos;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_families.size()); // Evil statement
        device_create_info.pEnabledFeatures = &device_features;
#ifdef _ENABLE_COMPATIBILITY_WITH_OLDER_VK_IMPL
        // Enable compatibility with older Vulkan implementations: previous
        // implementations of Vulkan made a distinction between instance and
        // and device specific validation layers.
        if (enable_validation_layers) {
            device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            device_create_info.ppEnabledLayerNames = validation_layers.data();
        } else {
            device_create_info.enabledLayerCount = 0;
        }
        // Enable device extensions, required to use Vulkan
        // on your device
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        device_create_info.ppEnabledExtensionNames = device_extensions.data();
#endif // _ENABLE_COMPATIBILITY_WITH_OLDER_VK_IMPL
        if (vkCreateDevice(m_graphics_device, &device_create_info, nullptr, &m_logical_graphics_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device");
            return;
        }
        // Retrieve queue handles for each queue family (only one here)
        vkGetDeviceQueue(m_logical_graphics_device, queue_family_indices.graphics_family.value(), 0, &m_graphics_queue);
        vkGetDeviceQueue(m_logical_graphics_device, queue_family_indices.present_family.value(), 0, &m_present_queue);
    }
    
    void _createSwapChain() {
        Log("##########################");
        Log("Creating the swap chain...");
        Log("##########################");
        SwapChainSupportDetails swap_chain_support = querySwapChainSupport(m_graphics_device, m_surface);
        
        std::optional<VkSurfaceFormatKHR> surface_format = chooseSwapSurfaceFormat(swap_chain_support.formats);
        VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.present_modes);
        std::optional<VkExtent2D> extent = chooseSwapExtent(swap_chain_support.capabilities, m_app_window);
        
        if (!surface_format.has_value() || !extent.has_value()) {
            throw std::runtime_error("An error happened setting the swap chain support structure");
            return;
        }
        
        m_swap_chain_surface_format = surface_format.value();
        m_swap_chain_extent = extent.value();
        
        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
        // Make sure to not exceed the max number of images here
        if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount)
            image_count = swap_chain_support.capabilities.maxImageCount;
        
        Log("-> Image count: " << image_count);
        
        VkSwapchainCreateInfoKHR swap_chain_create_info {};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = m_surface; // Specify which surface the swap chain should be tied to
        swap_chain_create_info.minImageCount = image_count;
        swap_chain_create_info.imageFormat = m_swap_chain_surface_format.format;
        swap_chain_create_info.imageColorSpace = m_swap_chain_surface_format.colorSpace;
        swap_chain_create_info.imageExtent = m_swap_chain_extent;
        swap_chain_create_info.imageArrayLayers = 1; // Always one here, as we do not develop something with 3D
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Color attachment only
        
        // As graphics queue != present queue, we need to specify how to handle swap chain images
        // that will be used across multiple queue families
        QueueFamilyIndices indices = findQueueFamilies(m_graphics_device, m_surface);
        uint32_t family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};
        if (indices.graphics_family != indices.present_family) {
            Log("-> Graphics and Present family queues are different");
            // No explicit ownership transfers with images that can be used
            // across multiple queue families
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount = 2;
            swap_chain_create_info.pQueueFamilyIndices = family_indices;
        } else {
            Log("-> Graphics and Present family queues are the same");
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        // No transformation
        swap_chain_create_info.preTransform = swap_chain_support.capabilities.currentTransform;
        // Ignore the alpha channel
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = present_mode;
        swap_chain_create_info.clipped = VK_TRUE; // Enable clipping
        swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE; // TODO: handle invalid / unoptimized swap chain at runtime

        Log("-> Initializing the swap chain... ");
        
        // Now that we configured the swap chain from scratch,
        // initialize it and store it
        if (vkCreateSwapchainKHR(m_logical_graphics_device, &swap_chain_create_info, nullptr, &m_swap_chain) != VK_SUCCESS) {
            Log("failed");
            throw std::runtime_error("failed to create the swap chain for the application");
        }
        // Retrieving the images from the swap chain
        uint32_t sw_images {};
        vkGetSwapchainImagesKHR(m_logical_graphics_device, m_swap_chain, &sw_images, nullptr);
        m_swap_chain_images.resize(sw_images);
        vkGetSwapchainImagesKHR(m_logical_graphics_device, m_swap_chain, &sw_images, m_swap_chain_images.data());
    }
    
    void _createImageViews() {
        Log("##################################");
        Log("Creating swap chain image views...");
        Log("##################################");
        m_swap_chain_image_views.resize(m_swap_chain_images.size());
        for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
            VkImageViewCreateInfo image_view_create_info {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = m_swap_chain_images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = m_swap_chain_surface_format.format;
            // swizzle the color channels
#ifndef __APPLE__
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_ONE;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_ONE;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_ONE;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_ONE;
#else
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
#endif
            // image's purpose, and which part of the image
            // should be accessed
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            
            Log("-> Image " << (i + 1) << " on " << m_swap_chain_images.size() << "... ");
            // Create the image now
            if (vkCreateImageView(m_logical_graphics_device, &image_view_create_info, nullptr, &m_swap_chain_image_views[i]) != VK_SUCCESS) {
                Log("failed");
                throw std::runtime_error("failed to create image views");
                break;
            }
        }
    }
    
    void _createRenderPass() {
        Log("#######################");
        Log("Creating render pass...");
        Log("#######################");
        VkAttachmentDescription color_attachment {};
        color_attachment.format = m_swap_chain_surface_format.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        // Clear the values to a constant at the start
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // Rendered contents will be stored in memory and can be read later
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference color_attachment_ref{};
        // Our array consists of a single VkAttachmentDescription, so its index is 0
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        
        VkSubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;
        
        if (vkCreateRenderPass(m_logical_graphics_device, &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    
    void _createGraphicsPipeline() {
        Log("#############################");
        Log("Creating graphics pipeline...");
        Log("#############################"); 
#ifdef __APPLE__
        const std::string vertex_shader_filepath = std::string(SHADERS_DIR).append("/").append("vert.spv");
        const std::string fragment_shader_filepath = std::string(SHADERS_DIR).append("/").append("frag.spv");
#elif defined _WIN32
        const std::string vertex_shader_filepath = std::string(SHADERS_DIR).append("\\").append("vert.spv");
        const std::string fragment_shader_filepath = std::string(SHADERS_DIR).append("\\").append("frag.spv");
#endif
        const auto opt_vertex_shader_code = loadShaderFile(vertex_shader_filepath);
        const auto opt_fragment_shader_code = loadShaderFile(fragment_shader_filepath);
        if (!opt_vertex_shader_code.has_value() || !opt_fragment_shader_code.has_value()) {
            std::cout << "vertex / fragment shader code have not been found - make sure to run 'build_shaders' script before";
        }
        assert(opt_vertex_shader_code.has_value() && opt_fragment_shader_code.has_value());
        assert(opt_vertex_shader_code.value().size() > 0 && opt_vertex_shader_code.value().size() == 1504);
        assert(opt_fragment_shader_code.value().size() > 0 && opt_fragment_shader_code.value().size() == 572);
        
        VkShaderModule vertex_shader_module {};
        {
            const std::vector<char> vertex_shader_code = opt_vertex_shader_code.value();
            VkShaderModuleCreateInfo shader_module_create_info {};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = vertex_shader_code.size();
            shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(vertex_shader_code.data());
            if (vkCreateShaderModule(m_logical_graphics_device, &shader_module_create_info, nullptr, &vertex_shader_module) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module for vertex shader!");
                return;
            }
        }
        VkPipelineShaderStageCreateInfo vert_shader_stage_info {};
        vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_stage_info.module = vertex_shader_module;
        vert_shader_stage_info.pName = "main"; // entrypoint - should be main by default
        vert_shader_stage_info.pSpecializationInfo = nullptr; // no configuration at pipeline creation
        
        VkShaderModule fragment_shader_module {};
        {
            const std::vector<char> fragment_shader_code = opt_fragment_shader_code.value();
            VkShaderModuleCreateInfo shader_module_create_info {};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = fragment_shader_code.size();
            shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(fragment_shader_code.data());
            if (vkCreateShaderModule(m_logical_graphics_device, &shader_module_create_info, nullptr, &fragment_shader_module) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module for fragment shader!");
                return;
            }
        }
        VkPipelineShaderStageCreateInfo frag_shader_stage_info {};
        frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_info.module = fragment_shader_module;
        frag_shader_stage_info.pName = "main";
        frag_shader_stage_info.pSpecializationInfo = nullptr;
        
        // Now, create the graphics pipeline stages...
        VkPipelineShaderStageCreateInfo shader_stages[] = {
            vert_shader_stage_info,
            frag_shader_stage_info
        };
        
        // Vertex input setup
        VkPipelineVertexInputStateCreateInfo vertex_input_info {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 0;
        vertex_input_info.pVertexBindingDescriptions = nullptr;
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions = nullptr;
        
        // Input assembly setup
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info {};
        input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO: change to POINTS_LIST ?
        input_assembly_info.primitiveRestartEnable = VK_FALSE;
        
        // Viewport and scissoring
        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.height = m_swap_chain_extent.height;
        viewport.width = m_swap_chain_extent.width;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        // Cover the viewport entirely
        VkRect2D scissor {};
        scissor.offset = {0, 0};
        scissor.extent = m_swap_chain_extent;
        // Combine both in a viewport state
        VkPipelineViewportStateCreateInfo viewport_state {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;
        
        // Rasterizer configuration
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
        rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth = 1.0f;
        rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable = VK_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
        
        // Multisampling
        // The multisampling is a way to perform anti-aliasing
        VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable = VK_FALSE;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_state_create_info.minSampleShading = 1.0f;
        multisample_state_create_info.pSampleMask = nullptr;
        multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
        multisample_state_create_info.alphaToOneEnable = VK_FALSE;
        
        // Color blinding
        VkPipelineColorBlendAttachmentState color_blend_attachment {};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;
        
        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        
        // Dynamic state
        std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };
        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();
        
        // Pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_info {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;
        
        if (vkCreatePipelineLayout(m_logical_graphics_device, &pipeline_layout_info, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create a pipeline layout");
        }
        
        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly_info;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterization_state_create_info;
        pipeline_info.pMultisampleState = &multisample_state_create_info;
        pipeline_info.pDepthStencilState = nullptr; // Optional
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = nullptr; // Optional
        pipeline_info.layout = m_pipeline_layout;
        pipeline_info.renderPass = m_render_pass;
        pipeline_info.subpass = 0;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;
        
        if (vkCreateGraphicsPipelines(m_logical_graphics_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        
        // Cleanup the modules
        vkDestroyShaderModule(m_logical_graphics_device, vertex_shader_module, nullptr);
        vkDestroyShaderModule(m_logical_graphics_device, fragment_shader_module, nullptr);
    }
    
    void _createFramebuffers() {
        Log("########################");
        Log("Creating framebuffers...");
        Log("########################");
        m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());
        for (size_t i = 0; i < m_swap_chain_image_views.size(); i++) {
            VkImageView attachments[] = {
                m_swap_chain_image_views[i]
            };
            VkFramebufferCreateInfo framebuffer_info {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = m_render_pass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
            framebuffer_info.width = m_swap_chain_extent.width;
            framebuffer_info.height = m_swap_chain_extent.height;
            framebuffer_info.layers = 1; // swap chain images are single images
            
            if (vkCreateFramebuffer(m_logical_graphics_device, &framebuffer_info, nullptr, &m_swap_chain_framebuffers[i]) != VK_SUCCESS) {
                LogE("failed to create framebuffer at index " << i);
                throw std::runtime_error("failed to create framebuffer");
            }
        }
    }
    
    void _createCommandPool() {
        Log("########################");
        Log("Creating command pool...");
        Log("########################");
        QueueFamilyIndices queue_family_indices = findQueueFamilies(m_graphics_device, m_surface);
        VkCommandPoolCreateInfo command_pool_create_info {};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // allow command buffers to be rerecorded individually, to
        // record a command buffer every frame
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // graphics_family has a value here, otherwise it might fail
        // way before the command pool creation
        command_pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
        // allow to draw
        if (vkCreateCommandPool(m_logical_graphics_device, &command_pool_create_info, nullptr, &m_command_pool) != VK_SUCCESS) {
            LogE("failed to create command pool!");
            throw std::runtime_error("failed to create command pool!");
        }
    }
    
    void _createCommandBuffer() {
        Log("##########################");
        Log("Creating command buffer...");
        Log("##########################");
        VkCommandBufferAllocateInfo command_buffer_alloc_info {};
        command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_alloc_info.commandPool = m_command_pool;
        // VK_COMMAND_BUFFER_LEVEL_PRIMARY = can be submitted to a queue for
        // execution, but cannot be called from other command buffers.
        command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_alloc_info.commandBufferCount = 1;
        
        if (vkAllocateCommandBuffers(m_logical_graphics_device, &command_buffer_alloc_info, &m_command_buffer) != VK_SUCCESS) {
            Log("failed to create command buffer!");
            throw std::runtime_error("failed to create command buffer!");
        }
    }
    
    void _createSyncObjects() {
        Log("########################");
        Log("Creating sync objects...");
        Log("########################");
        // Semaphore setup
        VkSemaphoreCreateInfo semaphore_create_info {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        // Fence setup
        VkFenceCreateInfo fence_create_info {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // The VK_FENCE_CREATE_SIGNALED_BIT flag set the
        // "Signaled" signal at first as, at the first
        // drawFrame call, there is no frame previously.
        // Using this flag, the program will not wait forever for
        // an image that does not exist...
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateSemaphore(m_logical_graphics_device, &semaphore_create_info, nullptr, &m_image_avail_semaphore) != VK_SUCCESS) {
            LogE("failed to create semaphore for image availability");
            throw std::runtime_error("failed to create semaphore for image availability");
            return;
        }
        if (vkCreateSemaphore(m_logical_graphics_device, &semaphore_create_info, nullptr, &m_render_finished_semaphore) != VK_SUCCESS) {
            LogE("failed to create semaphore for finished render");
            throw std::runtime_error("failed to create semaphore for finished render");
            return;
        }
        if (vkCreateFence(m_logical_graphics_device, &fence_create_info, nullptr, &m_in_flight_fence) != VK_SUCCESS) {
            LogE("failed to create the fence for image synchronization");
            throw std::runtime_error("failed to create the fence for image synchronization");
        }
    }
    
    void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index) {
        VkCommandBufferBeginInfo command_buffer_begin_info {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
            Log("failed to begin command buffer!");
            throw std::runtime_error("failed to begin command buffer!");
            return;
        }
        
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_render_pass;
        render_pass_info.framebuffer = m_swap_chain_framebuffers[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = m_swap_chain_extent;
        
        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;
        
        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
        vkCmdDraw(command_buffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(command_buffer);
        
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            Log("failed to record command buffer!");
            throw std::runtime_error("failed to record command buffer!");
            return;
        }
    }
    
    void drawFrame() {
        // Wait until the previous frame has finished
        vkWaitForFences(m_logical_graphics_device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_logical_graphics_device, 1, &m_in_flight_fence);
        
        // Acquire an image from the swap chain
        uint32_t image_acq_index {};
        vkAcquireNextImageKHR(
                              m_logical_graphics_device,
                              m_swap_chain,
                              UINT64_MAX,
                              m_image_avail_semaphore,
                              VK_NULL_HANDLE,
                              &image_acq_index);
        vkResetCommandBuffer(m_command_buffer, 0);
        recordCommandBuffer(m_command_buffer, image_acq_index);
        
        VkSemaphore wait_semaphores[] = {m_image_avail_semaphore};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signal_semaphores[] = {m_render_finished_semaphore};
        
        // Submit the command buffer
        VkSubmitInfo submit_info {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;
        
        if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fence) != VK_SUCCESS) {
            LogE("failed to submit draw command buffer!");
            throw std::runtime_error("failed to submit draw command buffer!");
            return;
        }
        
        VkPresentInfoKHR present_info{};
        VkSwapchainKHR swap_chains[] = {m_swap_chain};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swap_chains;
        present_info.pImageIndices = &image_acq_index;
        vkQueuePresentKHR(m_present_queue, &present_info);
    }
    
    void initWindow() {
        glfwInit(); // Initialize the GLFW library
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resizable option for the window
        // The last parameter in glfwCreateWindow is only for OpenGL - no need to setup it here
        // TODO: Set the monitor
        m_app_window = glfwCreateWindow(WIDTH, HEIGHT, APPLICATION_TITLE, nullptr, nullptr);
    }
    
    void initSystem() {
        _initVulkan();
        _createSurface();
        _pickGraphicsDevice();
        _initLogicalGraphicsDevice();
        _createSwapChain();
        _createImageViews();
        _createRenderPass();
        _createGraphicsPipeline();
        _createFramebuffers();
        _createCommandPool();
        _createCommandBuffer();
        _createSyncObjects();
    }
    
    void loop() {
        while (!glfwWindowShouldClose(m_app_window)) {
            glfwPollEvents();
            drawFrame();
        }
        // Clean up resources before ending the app
        vkDeviceWaitIdle(m_logical_graphics_device);
    }
    
    void clean() {
        Log("######################################");
        Log("Ending and cleaning the application...");
        Log("######################################");
        
        Log("* Destroying semaphores and fence objects...");
        vkDestroySemaphore(m_logical_graphics_device, m_image_avail_semaphore, nullptr);
        vkDestroySemaphore(m_logical_graphics_device, m_render_finished_semaphore, nullptr);
        vkDestroyFence(m_logical_graphics_device, m_in_flight_fence, nullptr);
        
        Log("* Destroying the command pool...");
        vkDestroyCommandPool(m_logical_graphics_device, m_command_pool, nullptr);
        
        Log("* Destroying the framebuffers...");
        for (const auto framebuffer: m_swap_chain_framebuffers) {
            vkDestroyFramebuffer(m_logical_graphics_device, framebuffer, nullptr);
        }
        
        Log("* Destroying the graphics pipeline...");
        vkDestroyPipeline(m_logical_graphics_device, m_graphics_pipeline, nullptr);
        
        Log("* Destroying the pipeline layout...");
        vkDestroyPipelineLayout(m_logical_graphics_device, m_pipeline_layout, nullptr);
        
        Log("* Destroying the render pass...");
        vkDestroyRenderPass(m_logical_graphics_device, m_render_pass, nullptr);

        Log("* Destroying the image views...");
        for (const VkImageView image_view: m_swap_chain_image_views) {
            vkDestroyImageView(m_logical_graphics_device, image_view, nullptr);
        }
        
        Log("* Destroying the swap chain...");
        if (m_swap_chain != NULL) vkDestroySwapchainKHR(m_logical_graphics_device, m_swap_chain, nullptr);

        Log("* Destroying the logical device...");
        if (m_logical_graphics_device != NULL) vkDestroyDevice(m_logical_graphics_device, nullptr);
        
        Log("* Destroying the Vulkan surface...");
        if (m_surface != NULL) vkDestroySurfaceKHR(m_vk_instance, m_surface, nullptr);

        Log("* Destroying the Vulkan instance...");
        if (m_vk_instance != NULL) vkDestroyInstance(m_vk_instance, nullptr);
        
        Log("* Destroying the (GLFW) window...");
        if (m_app_window != nullptr) glfwDestroyWindow(m_app_window);

        Log("Terminating...");
        glfwTerminate();
    }
};

int main() {
    TriangleApplication app;
    
    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
