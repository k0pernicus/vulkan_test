//
//  Base.h
//  VulkanTest
//
//  Created by Antonin on 21/06/2022.
//

#ifndef Base_h
#define Base_h

#include <iostream>

#ifdef __APPLE__
#pragma message "Apple platform support"
#elif defined _WIN32
#pragma message "Microsoft Windows platform support"
#endif

#ifdef DEBUG
#define Log(x) std::cout << x << std::endl
#define LogE(x) std::cerr << x << std::endl
#else
// Not sure this Log function is correct...
#define Log(x)
#define LogE(x) std::cerr << x << std::endl
#endif

#endif /* Base_h */
