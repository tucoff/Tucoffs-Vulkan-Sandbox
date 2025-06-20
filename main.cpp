/*
 * Vulkan Program
 *
 * Copyright (C) 2016 Valve Corporation
 * Copyright (C) 2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
Vulkan Project Template
Create and destroy a simple Vulkan instance.
*/

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

int screen()
{
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // Adicione isso antes de glfwInit()
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkanwindow", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    std::cout << extensionCount << "extensions supported\n";
    
    glm::mat4x4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;
    
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
    } 

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

#include "epic_triangle.h"

int main()
{    
    //This is the code for the GLFW window with Vulkan support and inicial testing
    //screen();
    
    //This was the template code for the Vulkan project
    //project(); 

    // With this treasure, I summon Three-Handled Sword Divergent Sila Divine General T R I A N G U L O
    triangle();

    return 0;
}
