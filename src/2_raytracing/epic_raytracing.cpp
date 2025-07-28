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

// Region: Structs
// This section defines structs used to hold Vulkan-related data, such as queue family indices and swap chain support details.
# pragma region Structs

// A struct to hold the indices of queue families found on a physical device.
// std::optional is used because a queue family might not be found.
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily; // Index of the queue family that supports graphics operations.
    std::optional<uint32_t> presentFamily; // Index of the queue family that supports presenting images to a surface (e.g., window).

    // Checks if all required queue families have been found.
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value(); // Returns true if a graphics family and a present family index are present.
    }
};

// A struct to hold the details of swap chain support for a physical device.
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities = {}; // Surface capabilities, such as image count and size limits.
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
#pragma endregion

class HelloRayTracingApplication
{
public:
    void run()
    {
        initWindow();     // Initialize the GLFW window.
        initVulkan();     // Initialize Vulkan components.

        // Main loop: mantém a janela aberta até o usuário fechar
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }

        cleanup();        // Clean up Vulkan and GLFW resources.
    }

private:
    // Private member variables for the application state.
    // GLFW window pointer, Vulkan instance, debug messenger, surface, physical device, logical device
    // queues, swap chain, and other Vulkan objects.
    // These variables are used to manage the Vulkan rendering pipeline and resources.
    #pragma region Private Member Variables
    GLFWwindow* window = nullptr;                                   // Pointer to the GLFW window object.
    VkInstance instance = VK_NULL_HANDLE;                           // Vulkan instance handle.
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;       // Vulkan debug messenger object.
    VkSurfaceKHR surface = VK_NULL_HANDLE;                          // Vulkan surface for rendering to the window.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;               // Handle to the selected physical device (GPU). 
    VkPhysicalDeviceFeatures supportedPhysicalDeviceFeatures = {};  // Structure to hold the features supported by the physical device.
    VkDevice device = VK_NULL_HANDLE;                               // Vulkan logical device object.
    VkQueue graphicsQueue = VK_NULL_HANDLE;                         // Handle to the graphics queue.
    VkQueue presentQueue = VK_NULL_HANDLE;                          // Handle to the present queue (for displaying images on the surface).
    #pragma endregion

    // Initializes the GLFW window and sets up the necessary callbacks.
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

    // Initializes Vulkan components and sets up the rendering pipeline.
    #pragma region InitVulkan()
    // Initializes Vulkan components.
    void initVulkan()
    {
        createInstance();        // Create the Vulkan instance.
        setupDebugMessenger();   // Set up the debug messenger for validation layers.
        createSurface();         // Create a Vulkan surface for rendering.
        pickPhysicalDevice();    // Select a suitable physical device (GPU).
        createLogicalDevice();   // Create the logical device.
        // Note: Additional Vulkan setup steps like swap chain, pipelines, etc. would go here
    }
    
    // Creates the Vulkan instance.
    void createInstance()
    {
        // Check if validation layers are requested but not supported by the system.
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // Populate application information. This is optional but good practice.
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // Specifies the type of the structure.
        appInfo.pApplicationName = "Traçando Raio";             // Name of the application
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Application version (1.0.0).
        appInfo.pEngineName = "LXXIV";                          // Name of the engine, yeah its a me reference.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // Engine version (1.0.0).
        appInfo.apiVersion = VK_API_VERSION_1_0;                // Vulkan API version used.

        // Populate instance creation information.
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // Specifies the type of the structure.
        createInfo.pApplicationInfo = &appInfo;                    // Pointer to the application info struct.

        // Get the required Vulkan extensions based on GLFW and validation layer settings.
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // Number of enabled extensions.
        createInfo.ppEnabledExtensionNames = extensions.data();                      // Pointer to an array of extension names.

        // Handle validation layers setup.
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // Structure for debug messenger creation info.
        if (enableValidationLayers)
        {
            // Enable validation layers.
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); // Number of enabled layers.
            createInfo.ppEnabledLayerNames = validationLayers.data();                      // Pointer to an array of layer names.

            // Populate the debug messenger create info and chain it to the instance creation info.
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo; // Chain to the next structure.
        }
        else
        {
            // If validation layers are disabled, set layer count to 0 and pNext to nullptr.
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // Attempt to create the Vulkan instance.
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!"); // Throw an error if instance creation fails.
        }
    }

    // Checks if all required validation layers are supported by the Vulkan instance.
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        // Get the number of available instance layers.
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        // Allocate a vector to hold the properties of all available layers.
        std::vector<VkLayerProperties> availableLayers(layerCount);
        // Get the properties of all available instance layers.
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Iterate through each required validation layer.
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            // Check if the current required layer is present in the available layers.
            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0) // Compare layer names.
                {
                    layerFound = true; // Set flag if layer is found.
                    break;             // No need to check further for this layer.
                }
            }

            // If a required layer was not found, return false immediately.
            if (!layerFound)
            {
                return false;
            }
        }

        return true; // All required validation layers are supported.
    }

    // Retrieves the list of required Vulkan instance extensions.
    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        // Get the extensions required by GLFW for window surface creation.
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // Create a vector from the GLFW required extensions.
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // If validation layers are enabled, add the debug utility extension.
        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions; // Return the combined list of required extensions.
    }

    // Populates the VkDebugUtilsMessengerCreateInfoEXT structure.
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {}; // Initialize the structure to all zeros.
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;     // Specifies the type of the structure.

        // Define the message severities that will trigger the callback.
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |  // Detailed diagnostic messages.
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |                           // Potential problems.
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;                              // Invalid operations.
        
        // Define the message types that will trigger the callback.
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |          // General events.
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |                            // Violations of Vulkan spec or best practices.
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;                            // Potential performance issues.
        
        // Pointer to the callback function.
        createInfo.pfnUserCallback = debugCallback; 

        // pUserData can be used to pass arbitrary data to the callback, but it's not used here.
        // createInfo.pUserData = nullptr;
    }

    // Static callback function for Vulkan debug messages.
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        // Print the validation layer message to the error stream.
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE; // Return VK_FALSE to indicate that the Vulkan call should not be aborted.
    }

    // Sets up the debug messenger for Vulkan validation layers.
    void setupDebugMessenger()
    {
        // If validation layers are not enabled, return early.
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        // Populate the create info structure for the debug messenger.
        populateDebugMessengerCreateInfo(createInfo);

        // Attempt to create the debug messenger.
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!"); // Throw an error if setup fails.
        }
    }

    // Creates a Vulkan surface for rendering to the GLFW window.
    void createSurface() 
    {
        // Check if the GLFW window is valid before creating the surface.
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create window surface!"); // Throw an error if surface creation fails.
        }
    }

    // Selects a suitable physical device (GPU) for Vulkan operations.
    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        // Query the number of available physical devices.
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // If no physical devices are found, throw an error.
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // Allocate a vector to hold handles to all physical devices.
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // Get the handles to all physical devices.
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Iterate through the available devices to find a suitable one.
        for (const auto &device : devices) {
            VkPhysicalDeviceFeatures currentDeviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &currentDeviceFeatures); // Query here!

            // You might add checks here if you *need* specific features to be present
            // if (currentDeviceFeatures.samplerAnisotropy && isDeviceSuitable(device)) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                vkGetPhysicalDeviceFeatures(device, &supportedPhysicalDeviceFeatures); // Store ALL supported features
                break;
            }
        }

        // If no suitable physical device was found, throw an error.
        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // Checks if the given physical device is suitable for the application.
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device); // Find the queue families supported by the device.
        bool extensionsSupported = checkDeviceExtensionSupport(device); // Check if the required device extensions are supported.

        // Check if the swap chain is adequate for the device.
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // Query device features and store them if suitable
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures); // Query here!

        // Ensure the device supports the features you *plan* to enable.
        // For now, since you're requesting none, this check will always pass.
        // If you enable features later, you'd add checks like:
        // bool requiredFeaturesSupported = deviceFeatures.samplerAnisotropy;

        return indices.isComplete() && extensionsSupported && swapChainAdequate; // && requiredFeaturesSupported;
    }

    // Checks if the required Vulkan validation layers are supported by the system.
    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // Get the number of available device extensions.
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        // If no extensions are available, return false.
        if (extensionCount == 0) return false;
        // Allocate a vector to hold the properties of all available extensions.
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        // Get the properties of all available device extensions.
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        // Create a set from the required device extensions for easy lookup.
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        // Iterate through each available extension.
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName); // Remove found extensions from the set.
        }
        // If the set is empty, all required extensions are supported.
        return requiredExtensions.empty();
    }

    // Queries the swap chain support details for a given physical device.
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) 
    {
        // Create a SwapChainSupportDetails structure to hold the details.
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
      
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        
        if (formatCount != 0) 
        {
            details.formats.resize(formatCount);
            
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        
        if (presentModeCount != 0) 
        {
            details.presentModes.resize(presentModeCount);
            
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());            
        }

        return details;
    }

    // Creates the Vulkan logical device.
    void createLogicalDevice()
    {
        // Find the required queue families (e.g., graphics queue).
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // Create a vector to hold queue creation info structures
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        // Use a set to ensure unique queue family indices (graphics and present)
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        // For each unique queue family, fill out the queue creation info
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures enabledFeatures = {};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &enabledFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());;
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // Enable validation layers if requested
        if (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
        }

        // Create the logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create logical device!");
        }

        // Retrieve the handles for the graphics and present queues
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    // Finds the queue families supported by a given physical device.
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices; // Structure to store the found queue family indices.

        uint32_t queueFamilyCount = 0;
        // Get the number of queue families supported by the device.
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // Allocate a vector to hold the properties of all queue families.
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // Get the properties of all queue families.
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0; // Index for iterating through queue families.
        // Iterate through each queue family to find the required ones.
        for (const auto& queueFamily : queueFamilies)
        {
            // Check if the queue family supports graphics operations.
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i; // Store the index if it supports graphics.
            }

            // Check if the queue family supports presenting images to a surface.
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            // If the queue family supports presenting, store its index.
            if (presentSupport) 
            {
                indices.presentFamily = i; // Store the index
            }

            // If all required queue families have been found, stop searching.
            if (indices.isComplete())
            {
                break;
            }

            i++; // Increment the index.
        }

        return indices; // Return the found queue family indices.
    }

    #pragma endregion

    // Cleans up Vulkan and GLFW resources.
    #pragma region Cleanup()
    // Cleans up Vulkan and GLFW resources.
    void cleanup()
    {
        // Destroy the GLFW window.
        if (window != nullptr) {
            glfwDestroyWindow(window);
            window = nullptr; // Set the window pointer to nullptr after destruction.
        }

        // Terminate GLFW.
        glfwTerminate();
    }
    #pragma endregion

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