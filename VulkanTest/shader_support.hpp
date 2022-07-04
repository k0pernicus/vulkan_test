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
#ifdef _WIN32
#include <string>
#endif

#ifdef __APPLE__
constexpr char SHADERS_DIR[] = "./shaders";
#elif defined _WIN32
constexpr char SHADERS_DIR[] = "..\\..\\shaders";
#endif

std::optional<std::vector<char>> loadShaderFile(const std::string filename);

#include <stdio.h>

#endif /* shader_support_hpp */
