// Region: Includes
// This section includes necessary headers and defines platform-specific macros for Vulkan and GLFW.
// It also sets up the application window and prepares for Vulkan initialization.
// The code is structured to be compatible with both Windows and Linux platforms, using conditional compilation.
#pragma region Includes

// epic_raytracing.cpp
#include "epic_raytracing.h"          // Include the header file for this module
#include "utils.h"                    // Include the header file for this module

#if defined(_WIN32) || defined(_WIN64) // Check if the platform is Windows
    #define VK_USE_PLATFORM_WIN32_KHR
    #define PLATFORM_WINDOWS
#elif defined(__linux__) // Check if the platform is Linux
    #define VK_USE_PLATFORM_XCB_KHR
    #define PLATFORM_LINUX
#endif

#define GLFW_INCLUDE_VULKAN         // Define this to include Vulkan-specific headers with GLFW
#include <GLFW/glfw3.h>             // Include GL framework for window management and input handling
#include <iostream>                 // For standard input/output operations (e.g., std::cerr)
#include <fstream>                  // For file stream operations (not used in this code, but often included for file I/O)
#include <stdexcept>                // For standard exception handling (e.g., std::runtime_error)
#include <vector>                   // For using std::vector dynamic arrays
#include <cstring>                  // For C-style string manipulation (e.g., strcmp)
#include <cstdlib>                  // For general utilities (e.g., EXIT_SUCCESS, EXIT_FAILURE)
#include <optional>                 // For using std::optional to represent potentially absent values
#include <set>                      // For using std::set to store unique values
#include <cstdint>                  //Necessary for uint32_t
#include <limits>                   //Necessary for std::numeric_limits
#include <algorithm>                //Necessary for std::clamp

#pragma endregion

// Region: Constants and Global Variables
// This section defines constants and global variables used throughout the application.
#pragma region Constants and Global Variables

// Define the width of the application window
const uint32_t WIDTH = 800;
// Define the height of the application window
const uint32_t HEIGHT = 600;

// A vector of C-style strings containing the names of Vulkan validation layers to enable.
// These layers provide debugging and error checking for Vulkan API usage.
const std::vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

// A vector of C-style strings containing the names of Vulkan device extensions to enable.
const std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Conditional compilation to enable or disable validation layers based on the build configuration.
// If NDEBUG (No Debug) is defined, validation layers are disabled for release builds.
// Otherwise, they are enabled for debug builds.
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#pragma endregion

class HelloRayTracingApplication
{
public:
    void run()
    {
        initWindow();     // Initialize the GLFW window.
    }

private:
    // Private member variables for the application state.
    // GLFW window pointer, Vulkan instance, debug messenger, surface, physical device, logical device
    // queues, swap chain, and other Vulkan objects.
    // These variables are used to manage the Vulkan rendering pipeline and resources.
    #pragma region Private Member Variables
    GLFWwindow* window = nullptr;                                   // Pointer to the GLFW window object.
    VkInstance instance = VK_NULL_HANDLE;                           // Vulkan instance handle.
    #pragma endregion

    #pragma region InitWindow()
    // Initializes the GLFW window.
    void initWindow()
    {
        #if defined(_WIN32) || defined(_WIN64)
            // No platform hint needed for Windows
        #elif defined(__linux__)
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // Set X11 platform for Linux
        #endif
        glfwInit(); // Initialize the GLFW library.

        // Tell GLFW not to create an OpenGL context, as we are using Vulkan.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Disable window resizing initially, as it simplifies initial Vulkan setup.
        // The callback will handle resizing later.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Removed to allow resizing

        // Create the GLFW window with specified width, height, title, and no full-screen or sharing.
        window = glfwCreateWindow(WIDTH, HEIGHT, "Traçando Raio", nullptr, nullptr);

        // Set the user pointer for the window to this HelloTriangleApplication instance,
        // allowing access to its members from static callbacks.
        glfwSetWindowUserPointer(window, this);
    }
    # pragma endregion

};

// Create a EPIC EXTREME ULTRA MEGA standard FUDEROSO ray tracing in Vulkan
// This is the main function where the application execution begins.
int raytrace()
{
    HelloRayTracingApplication app; // Create an instance of the HelloRayTracingApplication class.

    try
    {
        app.run(); // Run the Vulkan application.
    }
    catch (const std::runtime_error& e) // Catch any std::runtime_error exceptions.
    {
        std::cerr << "Error: " << e.what() << std::endl; // Print the error message to the error stream.
        return EXIT_FAILURE; // Return a failure code.
    }

    return EXIT_SUCCESS; // Return a success code if the application runs without errors.
}