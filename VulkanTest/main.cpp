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

#include "extension_support.hpp"
#include "queue_utils.hpp"
#include "swapchain_utils.hpp"

#ifdef __APPLE__
#pragma message "Apple platform support"
#else
error "Unsupported platform"
#endif

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
    // The retrieved and stored image format for our swap chain
    VkSurfaceFormatKHR m_swap_chain_surface_format;
    // The retrieved and stored extent of our swap chain
    VkExtent2D m_swap_chain_extent;
    
    VkApplicationInfo _createAppInfo() {
        // Create a Vulkan app info
        VkApplicationInfo app_info {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = APPLICATION_TITLE;
        app_info.applicationVersion = APP_VERSION;
        app_info.pEngineName = ENGINE_NAME;
        app_info.engineVersion = ENGINE_VERSION;
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
#ifdef DEBUG
        std::cout << "############################" << std::endl;
        std::cout << "Initializing Vulkan instance ..." << std::endl;
        std::cout << "############################" << std::endl;
#endif
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
            std::cout << "\t " << extension.extensionName << std::endl;
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
#ifdef DEBUG
        std::cout << "##########################" << std::endl;
        std::cout << "Creating window surface..." << std::endl;
        std::cout << "##########################" << std::endl;
#endif
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
#ifdef DEBUG
        std::cout << "#############################" << std::endl;
        std::cout << "Choosing a physical device..." << std::endl;
        std::cout << "#############################" << std::endl;
#endif
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
#ifdef DEBUG
        std::cout << "-> Checking the graphics device... ";
#endif
        if (m_graphics_device == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a graphice device (GPU) with Vulkan support");
            return;
        }
#ifdef DEBUG
        std::cout << "ok!" << std::endl;
        std::cout << "-> Checking the device extension support... ";
#endif
        if (!checkDeviceExtensionSupport(m_graphics_device)) {
            throw std::runtime_error("device extensions have not been found");
            return;
        }
#ifdef DEBUG
        std::cout << "ok!" << std::endl;
        std::cout << "-> Checking the swapchain support... ";
#endif
        // Make sure the SwapChain is adequate for our needs
        SwapChainSupportDetails swapchain_support = querySwapChainSupport(m_graphics_device, m_surface);
        if (swapchain_support.formats.empty() || swapchain_support.present_modes.empty()) {
            throw std::runtime_error("swapchain support is incorrect on your device");
            return;
        }
#ifdef DEBUG
        std::cout << "ok!" << std::endl;
#endif
    }
    
    /**
     * Based on the graphics device (or driver), initializes the logical device, and
     * the Queue Create Info information.
     */
    void _initLogicalGraphicsDevice() {
#ifdef DEBUG
        std::cout << "##########################" << std::endl;
        std::cout << "Init the logical device..." << std::endl;
        std::cout << "##########################" << std::endl;
#endif
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
#ifdef DEBUG
        std::cout << "##########################" << std::endl;
        std::cout << "Creating the swap chain..." << std::endl;
        std::cout << "##########################" << std::endl;
#endif
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

#ifdef DEBUG
            std::cout << "-> Image count: " << image_count << std::endl;
#endif
        
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
#ifdef DEBUG
            std::cout << "-> Graphics and Present family queues are different" << std::endl;
#endif
            // No explicit ownership transfers with images that can be used
            // across multiple queue families
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount = 2;
            swap_chain_create_info.pQueueFamilyIndices = family_indices;
        } else {
#ifdef DEBUG
            std::cout << "-> Graphics and Present family queues are the same" << std::endl;
#endif
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        // No transformation
        swap_chain_create_info.preTransform = swap_chain_support.capabilities.currentTransform;
        // Ignore the alpha channel
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = present_mode;
        swap_chain_create_info.clipped = VK_TRUE; // Enable clipping
        swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE; // TODO: handle invalid / unoptimized swap chain at runtime

#ifdef DEBUG
        std::cout << "-> Initializing the swap chain... ";
#endif
        
        // Now that we configured the swap chain from scratch,
        // initialize it and store it
        if (vkCreateSwapchainKHR(m_logical_graphics_device, &swap_chain_create_info, nullptr, &m_swap_chain) != VK_SUCCESS) {
#ifdef DEBUG
        std::cout << "failed" << std::endl;
#endif
            throw std::runtime_error("failed to create the swap chain for the application");
        }
#ifdef DEBUG
        std::cout << "success" << std::endl;
#endif
        // Retrieving the images from the swap chain
        uint32_t sw_images {};
        vkGetSwapchainImagesKHR(m_logical_graphics_device, m_swap_chain, &sw_images, nullptr);
        m_swap_chain_images.resize(sw_images);
        vkGetSwapchainImagesKHR(m_logical_graphics_device, m_swap_chain, &sw_images, m_swap_chain_images.data());
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
    }
    
    void loop() {
        while (!glfwWindowShouldClose(m_app_window)) glfwPollEvents();
    }
    
    void clean() {
#ifdef DEBUG
        std::cout << "######################################" << std::endl;
        std::cout << "Ending and cleaning the application..." << std::endl;
        std::cout << "######################################" << std::endl;
#endif
        // Destroy the Vulkan instances
#ifdef DEBUG
        std::cout << "* Destroying the swap chain..." << std::endl;
#endif
        if (m_swap_chain != NULL) vkDestroySwapchainKHR(m_logical_graphics_device, m_swap_chain, nullptr);
#ifdef DEBUG
        std::cout << "* Destroying the logical device..." << std::endl;
#endif
        if (m_logical_graphics_device != NULL) vkDestroyDevice(m_logical_graphics_device, nullptr);
#ifdef DEBUG
        std::cout << "* Destroying the Vulkan surface..." << std::endl;
#endif
        if (m_surface != NULL) vkDestroySurfaceKHR(m_vk_instance, m_surface, nullptr);
#ifdef DEBUG
        std::cout << "* Destroying the Vulkan instance..." << std::endl;
#endif
        if (m_vk_instance != NULL) vkDestroyInstance(m_vk_instance, nullptr);
        // Destroy the window
#ifdef DEBUG
        std::cout << "* Destroying the (GLFW) window..." << std::endl;
#endif
        if (m_app_window != nullptr) glfwDestroyWindow(m_app_window);
#ifdef DEBUG
        std::cout << "Terminating..." << std::endl;
#endif
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
