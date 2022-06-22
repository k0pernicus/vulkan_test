//
//  shader_support.hpp
//  VulkanTest
//
//  Created by Antonin on 19/06/2022.
//

#ifndef shader_support_hpp
#define shader_support_hpp

#include <vector>
#include <optional>

constexpr char SHADERS_DIR[] = "./shaders";

std::optional<std::vector<char>> loadShaderFile(const std::string filename);

#include <stdio.h>

#endif /* shader_support_hpp */
