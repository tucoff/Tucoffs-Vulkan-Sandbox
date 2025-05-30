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

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <windows.h> // Adiciona a definição de HMODULE e LoadLibraryA

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include<stdexcept>
#include<cstdlib>

#include "epic_triangle.h"

int screen()
{
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

int project()
{
    // Use validation layers if this is a debug build
    std::vector<const char*> layers;
    #if defined(_DEBUG)
        layers.push_back("VK_LAYER_KHRONOS_validation");
    #endif

    // VkApplicationInfo allows the programmer to specifiy some basic information about the
    // program, which can be useful for layers and tools to provide more debug information.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Tucoff's Sandbox";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "LXXIV";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // VkInstanceCreateInfo is where the programmer specifies the layers and/or extensions that
    // are needed. For now, none are enabled.
    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = NULL;
    instInfo.flags = 0;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = 0;
    instInfo.ppEnabledExtensionNames = NULL;
    instInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    instInfo.ppEnabledLayerNames = layers.data();

    // Create the Vulkan instance.
    VkInstance instance;
    VkResult result = vkCreateInstance(&instInfo, NULL, &instance);
    if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        std::cout << "Unable to find a compatible Vulkan Driver." << std::endl;
        return 1;
    }
    else if (result) {
        std::cout << "Could not create a Vulkan instance (for unknown reasons)." << std::endl;
        return 1;
    }

    // Normally, a program would do something with the instance here. This, however, is just a
    // simple demo program, so we just finish up right away.

    vkDestroyInstance(instance, NULL);

	return 0;
}

int main()
{    
	//This was the template code for the Vulkan project
	//project(); 

	//This is the code for the GLFW window with Vulkan support and inicial testing
	//screen();

	// With this treasure, I summon Three-Handled Sword Divergent Sila Divine General T R I A N G U L O
    triangle();

    return 0;
}
