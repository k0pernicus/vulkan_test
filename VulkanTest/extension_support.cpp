//
//  extension_support.cpp
//  VulkanTest
//
//  Created by Antonin on 17/06/2022.
//

#include "extension_support.hpp"

bool checkValidationLayerSupport() {
    uint32_t layer_count {};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    
    // Check for validation layers
    for (const char* layer_name : validation_layers) {
        bool found = false;
        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "ERROR: did not found the required validation layer(s)" << std::endl;
            return false;
        }
    }

    return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice physical_device) {
    uint32_t extensions_count {};
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr);
    
    std::vector<VkExtensionProperties> available_extensions(extensions_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, available_extensions.data());
    
    //Check for device extensions
    for (const char* extension_name: device_extensions) {
        bool found = false;
        for (const auto& extension_properties: available_extensions) {
#ifdef DEBUG
            std::cout << "Checking device extension " << extension_properties.extensionName << "... ";
#endif
            if (strcmp(extension_name, extension_properties.extensionName) == 0) {
                std::cout << "required!" << std::endl;
                found = true;
                break;
            } else
                std::cout << "**not** required!" << std::endl;
        }
        if (!found) {
            std::cerr << "ERROR: did not found the required device extension(s)" << std::endl;
            return false;
        }
    }
    
    return true;
}
