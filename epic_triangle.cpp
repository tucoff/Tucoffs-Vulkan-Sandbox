// epic_triangle.cpp
#include "epic_triangle.h"

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include<stdexcept>
#include<cstdlib>

class HelloTriangleApplication
{
public:
    void run()
    {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan()
    {

    }

    void mainLoop()
    {

    }

    void cleanup()
    {

    }
};

// Create a EPIC EXTREME ULTRA MEGA standard FUDEROSO triangle in Vulkan
int triangle()
{
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}