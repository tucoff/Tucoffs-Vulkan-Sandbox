// epic_triangle.cpp
#include "epic_triangle.h"          // Include the header file for this module
#define VK_USE_PLATFORM_XCB_KHR   // Define this to use the Win32 platform for Vulkan
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

// Define the width of the application window
const uint32_t WIDTH = 800;
// Define the height of the application window
const uint32_t HEIGHT = 600;

// Maximum number of frames that can be in flight (processed concurrently)
const int MAX_FRAMES_IN_FLIGHT = 2;

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

// Proxy function to create a debug utility messenger.
// vkCreateDebugUtilsMessengerEXT is an extension function, so its address must be loaded at runtime.
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    // Get the address of the vkCreateDebugUtilsMessengerEXT function from the Vulkan instance.
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    // If the function pointer is valid, call the function.
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    // Otherwise, return an error indicating the extension is not present.
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Proxy function to destroy a debug utility messenger.
// vkDestroyDebugUtilsMessengerEXT is an extension function, so its address must be loaded at runtime.
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    // Get the address of the vkDestroyDebugUtilsMessengerEXT function from the Vulkan instance.
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    // If the function pointer is valid, call the function.
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

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

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities = {}; // Surface capabilities, such as image count and size limits.
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Main application class for rendering a triangle with Vulkan.
class HelloTriangleApplication
{

public:
    // The main entry point for the application.
    void run()
    {
        initWindow();     // Initialize the GLFW window.
        initVulkan();     // Initialize Vulkan components.
        mainLoop();       // Enter the main application loop.
        cleanup();        // Clean up Vulkan and GLFW resources.
    }

private:
    GLFWwindow* window = nullptr;                                   // Pointer to the GLFW window object.
    VkInstance instance = VK_NULL_HANDLE;                           // Pointer to the GLFW window object.
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;       // Vulkan debug messenger object.
    VkSurfaceKHR surface = VK_NULL_HANDLE;                          // Vulkan surface for rendering to the window.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;               // Handle to the selected physical device (GPU). 
    VkPhysicalDeviceFeatures supportedPhysicalDeviceFeatures = {};  // Structure to hold the features supported by the physical device.
    VkDevice device = VK_NULL_HANDLE;                               // Vulkan logical device object.
    VkQueue graphicsQueue = VK_NULL_HANDLE;                         // Handle to the graphics queue.
    VkQueue presentQueue = VK_NULL_HANDLE;                          // Handle to the present queue (for displaying images on the surface).
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;                      // Vulkan swap chain for managing images to be presented.
    std::vector<VkImage> swapChainImages = {};                      // Vector to hold the images in the swap chain.
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;            // Format of the swap chain images.
    VkExtent2D swapChainExtent = {};                                // Extent (size) of the swap chain images.
    std::vector<VkImageView> swapChainImageViews = {};              // Vector to hold image views for the swap chain images.
    VkRenderPass renderPass = VK_NULL_HANDLE;                       // Vulkan render pass object for rendering operations.
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;               // Vulkan pipeline layout for the graphics pipeline.
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;                   // Vulkan graphics pipeline object for rendering.
    std::vector<VkFramebuffer> swapChainFramebuffers;               // Vector to hold framebuffers for the swap chain images.
    VkCommandPool commandPool = VK_NULL_HANDLE;                     // Vulkan command pool for managing command buffers.
    std::vector<VkCommandBuffer> commandBuffers;                    // Vector to hold command buffers for recording rendering commands.
    std::vector<VkSemaphore> imageAvailableSemaphores;              // Semaphores to signal when an image is available from the swap chain.
    std::vector<VkSemaphore> renderFinishedSemaphores;              // Semaphores to signal when rendering is finished for a frame.
    std::vector<VkFence> inFlightFences;                            // Fences to signal when a frame's rendering commands have finished executing.
    uint32_t currentFrame = 0;                                      // Index of the current frame being processed.

    bool framebufferResized = false;                                // Flag to indicate if the framebuffer has been resized.

    // Initializes the GLFW window.
    void initWindow()
    {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // Adicione isso antes de glfwInit()
        glfwInit(); // Initialize the GLFW library.

        // Tell GLFW not to create an OpenGL context, as we are using Vulkan.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Disable window resizing initially, as it simplifies initial Vulkan setup.
        // The callback will handle resizing later.
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Removed to allow resizing

        // Create the GLFW window with specified width, height, title, and no full-screen or sharing.
        window = glfwCreateWindow(WIDTH, HEIGHT, "Triangulo", nullptr, nullptr);

        // Set the user pointer for the window to this HelloTriangleApplication instance,
        // allowing access to its members from static callbacks.
        glfwSetWindowUserPointer(window, this);
        // Set the callback function for framebuffer size changes.
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    // Static callback function for GLFW framebuffer resize events.
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        // Retrieve the HelloTriangleApplication instance from the window user pointer.
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        // Set the framebufferResized flag to true to trigger swap chain recreation.
        app->framebufferResized = true;
    }

    // Initializes Vulkan components.
    void initVulkan()
    {
        createInstance();        // Create the Vulkan instance.
        setupDebugMessenger();   // Set up the debug messenger for validation layers.
        createSurface();         // Create a Vulkan surface for rendering.
        pickPhysicalDevice();    // Select a suitable physical device (GPU).
        createLogicalDevice();   // Create the logical device.
        createSwapChain();       // Create the swap chain for presenting images to the surface.
        createImageViews();      // Create image views for the swap chain images.
        createRenderPass();      // Create the render pass.
        createGraphicsPipeline();// Create the graphics pipeline
        createFramebuffers();    // Create the framebuffer
        createCommandPool();     // Create the command pool for command buffers.
        createCommandBuffers();  // Create command buffers for rendering commands.
        createSyncObjects();     // Create synchronization objects (semaphores and fences).
    }

    // The main application loop where events are polled and frames are drawn.
    void mainLoop()
    {
        // Loop as long as the window should not close (e.g., user clicks the close button).
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents(); // Process all pending GLFW events (e.g., keyboard input, mouse movement).
            drawFrame();      // Draw a single frame.
        }

        // Wait for the device to finish all pending operations before exiting.
        vkDeviceWaitIdle(device);
    }

    // Cleans up swap chain-related resources.
    void cleanupSwapChain() {
        // Destroy all framebuffers created for the swap chain.
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        // Destroy all image views created for the swap chain images.
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        // Destroy the Vulkan swap chain.
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    // Cleans up all allocated Vulkan and GLFW resources.
    void cleanup()
    {
        cleanupSwapChain(); // Call the function to clean up swap chain resources.

        // Destroy the graphics pipeline, pipeline layout, and render pass.
        vkDestroyPipeline(device, graphicsPipeline, nullptr); // Destroy the graphics pipeline.
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr); // Destroy the pipeline layout.
        vkDestroyRenderPass(device, renderPass, nullptr); // Destroy the render pass.

        // Destroy synchronization objects for each frame in flight.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr); // Destroy render finished semaphore.
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr); // Destroy image available semaphore.
            vkDestroyFence(device, inFlightFences[i], nullptr);               // Destroy in flight fence.
        }

        vkDestroyCommandPool(device, commandPool, nullptr); // Destroy the command pool.

        // Destroy the Vulkan logical device.
        vkDestroyDevice(device, nullptr);

        // Destroy the debug messenger if validation layers were enabled.
        if (enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        // Destroy the Vulkan surface associated with the GLFW window.
        vkDestroySurfaceKHR(instance, surface, nullptr);

        // Destroy the Vulkan instance.
        vkDestroyInstance(instance, nullptr);

        // Destroy the GLFW window.
        glfwDestroyWindow(window);

        // Terminate GLFW.
        glfwTerminate();
    }

    // Recreates the swap chain and related resources after a window resize or swap chain becoming out-of-date.
    void recreateSwapChain() {
        int width = 0, height = 0;
        // Get the current framebuffer size. If minimized, wait until it's not.
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents(); // Wait for events to process the window resize.
        }

        // Wait for the device to be idle before recreating resources that might be in use.
        vkDeviceWaitIdle(device);

        cleanupSwapChain(); // Clean up existing swap chain resources.

        createSwapChain();    // Create a new swap chain.
        createImageViews();   // Create new image views for the new swap chain images.
        createFramebuffers(); // Create new framebuffers for the new image views.
        // Command buffers don't need to be recreated because they don't depend on swap chain images directly,
        // only on the render pass and framebuffers, which are recreated.
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
        appInfo.pApplicationName = "Triangulo";                 // Name of the application and means "Triangle" from PT-BR.
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Application version (1.0.0).
        appInfo.pEngineName = "LXXIV";                          // Name of the engine.
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
        for (const auto& device : devices) {
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

    // Creates the swap chain for rendering images to the surface.
    void createSwapChain() 
    {
        // Query the swap chain support details for the selected physical device.
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats); // Choose the best surface format from the available formats.
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);  // Choose the best present mode from the available present modes.
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);                  // Choose the swap extent based on the surface capabilities.

        // Determine the number of images in the swap chain.
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        // If the maximum image count is specified and the calculated count exceeds it, use the maximum count.
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // Create the swap chain create info structure.
        VkSwapchainCreateInfoKHR createInfo{};                              // Initialize the structure to all zeros.
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;     // Specifies the type of the structure.
        createInfo.surface = surface;                                       // The surface to present images to (created earlier).
        createInfo.minImageCount = imageCount;                              // Set the minimum number of images in the swap chain.
        createInfo.imageFormat = surfaceFormat.format;                      // Set the image format for the swap chain.
        createInfo.imageColorSpace = surfaceFormat.colorSpace;              // Set the color space for the swap chain.
        createInfo.imageExtent = extent;                                    // Set the extent (size) of the swap chain images.
        createInfo.imageArrayLayers = 1;                                    // Set the number of layers in the swap chain images (1 for 2D images).
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;        // Set the usage of the swap chain images (color attachment for rendering).

        // Find the queue families for graphics and present operations.
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() }; // Create an array of queue family indices.
        
        if (indices.graphicsFamily != indices.presentFamily) // If the graphics and present queue families are different,
        {
            // Use concurrent mode for image sharing and allow images to be shared between the two families.
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; 
            createInfo.queueFamilyIndexCount = 2; 
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else // If they are the same,
        {    
            // Use exclusive mode for image sharing and do not share images between different queue families.
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;   // Use the current transform of the surface capabilities.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;              // Opaque composite alpha mode.
        createInfo.presentMode = presentMode;                                       // Set the present mode for the swap chain.
        createInfo.clipped = VK_TRUE;                                               // Enable clipping of images that are not visible.
        createInfo.oldSwapchain = VK_NULL_HANDLE;                                   // No previous swap chain, as this is the first one.

        // Attempt to create the swap chain with the specified parameters.
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!"); // Throw an error if swap chain creation fails.
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()); 
        
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
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

    // Chooses the best surface format from the available formats.
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
    {
        // If there is only one available format and it is VK_FORMAT_UNDEFINED, return it.
        for (const auto& availableFormat : availableFormats) 
        {
            // If the format is VK_FORMAT_UNDEFINED, it means no specific format is preferred.
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                return availableFormat;
            }
        }

        // If no preferred format is found, return the first available format.
        return availableFormats[0];
    }

    // Chooses the best present mode from the available present modes.
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        // Iterate through the available present modes to find the best one.
        for (const auto& availablePresentMode : availablePresentModes) 
        {
            // If mailbox mode is available, it is preferred for its low latency and triple buffering.
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                return availablePresentMode;
            }
        }

        // If mailbox mode is not available, use FIFO mode, which is the default and ensures images are presented in order.
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // Chooses the swap extent based on the surface capabilities.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        }
        else 
        {
            int width, height;
            
            glfwGetFramebufferSize(window, &width, &height);
                
            VkExtent2D actualExtent = 
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
               
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                
            return actualExtent;
        }
    }

    // Creates image views for the swap chain images.
    void createImageViews() 
    {
        // Resize the swapChainImageViews vector to match the number of swap chain images.
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) 
        {
            VkImageViewCreateInfo createInfo{};                                         // Initialize the structure to all zeros.
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;                // Specifies the type of the structure.
            createInfo.image = swapChainImages[i];                                      // The image to create the view for.
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                                // The type of the image view (2D in this case).
            createInfo.format = swapChainImageFormat;                                   // The format of the image view (same as swap chain image format).
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;                    // Identity swizzle for the red channel.
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;                    // Identity swizzle for the green channel.
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;                    // Identity swizzle for the blue channel.
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;                    // Identity swizzle for the alpha channel.
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;         // The aspect of the image to view (color in this case).
            createInfo.subresourceRange.baseMipLevel = 0;                               // The base mip level of the image view.
            createInfo.subresourceRange.levelCount = 1;                                 // The number of mip levels in the image view.
            createInfo.subresourceRange.baseArrayLayer = 0;                             // The base array layer of the image view.
            createInfo.subresourceRange.layerCount = 1;                                 // The number of array layers in the image view.

            // Attempt to create the image view for the swap chain image.
            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    // Creates the render pass for the application.
    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{}; // Create a description for the color attachment used in the render pass.
        colorAttachment.format = swapChainImageFormat; // Set the format of the color attachment to the swap chain image format.
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Use 1 sample per pixel (no multisampling).
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the attachment at the start
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store the result of rendering to the attachment.
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // No stencil operations at the start.
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // No stencil operations.
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // The initial layout of the attachment is undefined.
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // The final layout is the presentation layout for the swap chain.

        VkAttachmentReference colorAttachmentRef{}; // Create a reference to the color attachment.
        colorAttachmentRef.attachment = 0; // The index of the color attachment in the render pass.
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // The layout of the attachment during rendering.
        
        VkSubpassDescription subpass{}; // Create a description for the subpass in the render pass.
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // The subpass is for graphics pipelines.
        subpass.colorAttachmentCount = 1; // There is one color attachment in this sub
        subpass.pColorAttachments = &colorAttachmentRef; // Set the color attachment reference for the subpass.

        // Subpass dependency to ensure layout transitions happen correctly.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Refers to the implicit subpass before or after the render pass.
        dependency.dstSubpass = 0;                   // Refers to our subpass (index 0).
        // Operations that must happen before our subpass.
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        // Operations that will wait on the source.
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{}; // Create a structure to hold the render pass creation information.
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // Specifies the type
        renderPassInfo.attachmentCount = 1; // There is one attachment in the render pass.
        renderPassInfo.pAttachments = &colorAttachment; // Set the color attachment description.
        renderPassInfo.subpassCount = 1; // There is one subpass in the render pass.
        renderPassInfo.pSubpasses = &subpass; // Set the subpass description.
        renderPassInfo.dependencyCount = 1; // Number of subpass dependencies.
        renderPassInfo.pDependencies = &dependency; // Pointer to the subpass dependencies.

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create render pass!"); // Throw an error if render pass creation fails.
        }
    }

    // Creates the graphics pipeline for rendering.
    void createGraphicsPipeline()
    {
        // Read the SPIR-V shader code from files.
        auto vertShaderCode = readFile("../shaders/1_triangle/vert.spv");
        auto fragShaderCode = readFile("../shaders/1_triangle/frag.spv");

        // Create shader modules for the vertex and fragment shaders.
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // Create shader stage create info structures for the vertex shader.
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        // Create shader stage create info structure for the fragment shader.
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        // Combine the vertex and fragment shader stage create info into an array.
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        // Create the vertex input state create info structure.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        // Create the input assembly state create info structure.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Create the viewport state create info structure.
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Create the rasterization state create info structure.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Create the multisampling state create info structure.
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Create the color blend state create info structure.
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        // Create the color blending state create info structure.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // Define dynamic states that can be changed without recreating the pipeline.
        std::vector<VkDynamicState> dynamicStates = 
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        // Create the dynamic state create info structure.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Create the pipeline layout create info structure.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        // Create the pipeline layout.
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // Create the graphics pipeline create info structure.
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        // Create the graphics pipeline.
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        // Destroy the shader modules after the pipeline is created, as they are no longer needed.
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    // Creates a shader module from the provided SPIR-V code.
    VkShaderModule createShaderModule(const std::vector<char>& code) 
    {
        // Create a VkShaderModuleCreateInfo structure to hold the shader module creation parameters.
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        // Attempt to create the shader module using the Vulkan API.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        // Return the created shader module.
        return shaderModule;
    }

    // Creates framebuffers for each swap chain image view.
    void createFramebuffers()
    {
        // Resize the framebuffers vector to match the number of swap chain image views.
        swapChainFramebuffers.resize(swapChainImageViews.size());

        // Iterate through each swap chain image view to create a framebuffer.
        for (size_t i = 0; i < swapChainImageViews.size(); i++) 
        {
            VkImageView attachments[] = { swapChainImageViews[i] }; // Attach the current image view to the framebuffer.

            // Create the framebuffer create info structure.
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass; // The render pass to use with this framebuffer.
            framebufferInfo.attachmentCount = 1; // Number of attachments in the framebuffer.
            framebufferInfo.pAttachments = attachments; // The array of attachments.
            framebufferInfo.width = swapChainExtent.width; // Width of the framebuffer.
            framebufferInfo.height = swapChainExtent.height; // Height of the framebuffer.
            framebufferInfo.layers = 1; // Number of layers in the framebuffer.

            // Attempt to create the framebuffer using the Vulkan API.
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    // Creates a command pool for managing command buffers.
    void createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice); // Find the queue families for the physical device.

        // Create a command pool create info structure.
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be reset individually.
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // Use the graphics queue family index.

        // Attempt to create the command pool using the Vulkan API.
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    // Creates command buffers for recording rendering commands.
    void createCommandBuffers()
    {
        // Resize the command buffers vector to match MAX_FRAMES_IN_FLIGHT.
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        // Allocate command buffers from the command pool.
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size(); // Allocate enough command buffers.

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    // Records commands into a specific command buffer for rendering.
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        // Begin recording commands into the command buffer.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // No flags for this command buffer, as it's reset each frame.
        // beginInfo.flags = 0; 
        // beginInfo.pInheritanceInfo = nullptr; // Not used for primary command buffers.

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // Begin the render pass.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;                              // The render pass to use.
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];      // The framebuffer for the current swap chain image.
        renderPassInfo.renderArea.offset = {0, 0};                           // Offset for the render area.
        renderPassInfo.renderArea.extent = swapChainExtent;                  // Extent (size) for the render area.

        // Clear value for the color attachment (black, opaque).
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;         // Number of clear values.
        renderPassInfo.pClearValues = &clearColor;  // Pointer to the clear values.

        // Begin the render pass with inline command execution.
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Bind the graphics pipeline.
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            // Set the dynamic viewport.
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapChainExtent.width;
            viewport.height = (float) swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            // Set the dynamic scissor rectangle.
            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            // Draw command: 3 vertices, 1 instance, 0 first vertex, 0 first instance.
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        // End the render pass.
        vkCmdEndRenderPass(commandBuffer);

        // End recording commands into the command buffer.
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    // Creates synchronization objects (semaphores and fences) for frame management.
    void createSyncObjects() {
        // Resize vectors to hold synchronization objects for each frame in flight.
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        // Semaphore creation info.
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // Fence creation info, initialized as signaled to allow the first frame to proceed.
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Create in a signaled state.

        // Create semaphores and fences for each frame.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    // Draws a single frame of the application.
    void drawFrame() {
        // Wait for the fence of the current frame to be signaled (previous frame finished).
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        // Acquire an image from the swap chain. The imageAvailableSemaphore will be signaled when an image is ready.
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // Handle swap chain being out-of-date or suboptimal (e.g., window resized).
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain(); // Recreate the swap chain.
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // Reset the fence for the current frame before submitting new commands.
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        // Reset and record the command buffer for the current frame and image index.
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        // Submit information to the graphics queue.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Specify semaphores to wait on before execution.
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        // Specify the command buffer to execute.
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        // Specify semaphores to signal after execution.
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // Submit the command buffer to the graphics queue, signaling the fence upon completion.
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // Present information to the present queue.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // Specify semaphores to wait on before presenting.
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        // Specify the swap chains and image indices to present.
        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        // Queue the presentation operation.
        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        // Handle presentation results that require swap chain recreation.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false; // Reset the flag.
            recreateSwapChain();        // Recreate the swap chain.
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        // Advance to the next frame, wrapping around if MAX_FRAMES_IN_FLIGHT is reached.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

    // Reads a file and returns its contents as a vector of characters.
    static std::vector<char> readFile(const std::string& filename) 
    {
        // Open the file in binary mode and move the read pointer to the end to get the file size.
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        // Check if the file was opened successfully.
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        // Get the size of the file and create a buffer to hold its contents.
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        // Move the read pointer back to the beginning of the file and read its contents into the buffer.
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        // Close the file after reading its contents.
        file.close();

        return buffer; // Return the buffer containing the file contents.
    }

    // Static callback function for Vulkan debug messages.
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        // Print the validation layer message to the error stream.
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE; // Return VK_FALSE to indicate that the Vulkan call should not be aborted.
    }
};

// Create a EPIC EXTREME ULTRA MEGA standard FUDEROSO triangle in Vulkan
// This is the main function where the application execution begins.
int triangle()
{
    HelloTriangleApplication app; // Create an instance of the HelloTriangleApplication class.

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