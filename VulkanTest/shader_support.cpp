//
//  shader_support.cpp
//  VulkanTest
//
//  Created by Antonin on 19/06/2022.
//

#include "shader_support.hpp"
#include <fstream>

std::optional<std::vector<char>> loadShaderFile(const std::string filename) {
    // Read at the end to get the size
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("failed to open shader file " + std::string(filename));
        return std::nullopt;
    }
    
    // Instanciate the vector of characters to get the
    // content of the shader file
    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);
    
    // Once this has been allocated, read from the beginning
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    
    return buffer;
}
