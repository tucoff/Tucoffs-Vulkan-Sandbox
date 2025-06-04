// epic_triangle.cpp
#include "epic_triangle.h"

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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*>validationLayers = {
"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

bool checkValidationLayerSupport() 
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }

	return true;
}
    
class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    void createInstance()  
    {  
        if (enableValidationLayers && !checkValidationLayerSupport()) {  
            throw std::runtime_error("Validation layers requested, but not available!");  
		}

        VkApplicationInfo appInfo{};  
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;  
        appInfo.pApplicationName = "Triangulo";  
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  
        appInfo.pEngineName = "No Engine";  
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);  
        appInfo.apiVersion = VK_API_VERSION_1_0;  

        VkInstanceCreateInfo createInfo{};  
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;  
        createInfo.pApplicationInfo = &appInfo;  
        if (enableValidationLayers) {  
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());  
            createInfo.ppEnabledLayerNames = validationLayers.data();  
        } else {  
            createInfo.enabledLayerCount = 0;  
            createInfo.ppEnabledLayerNames = nullptr;  
		}

        uint32_t glfwExtensionCount = 0;  
        const char** glfwExtensions;  
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);  

        // --------------------------------------------------------------------------------
		// Uncomment the following lines if you want to check available Vulkan extensions
        //      uint32_t extensionCount = 0;
        //      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        //      std::vector<VkExtensionProperties> extensions(extensionCount);
        //      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        //      std::cout << "Available Vulkan extensions:" << std::endl;
        //      for (const auto& extension : extensions) {
        //          std::cout << "\t" << extension.extensionName << std::endl;
        //      }
        //
        //      // Check if the required extensions are available
        //      bool found = false;
        //      for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
        //          found = false;
        //          for (const auto& extension : extensions) {
        //              if (strcmp(glfwExtensions[i], extension.extensionName) == 0) {
        //                  found = true;
        //                  break;
        //              }
        //          }
        //          if (!found) {
        //              throw std::runtime_error("Required Vulkan extension not found: " + std::string(glfwExtensions[i]));
        //          }
        //      }
		// --------------------------------------------------------------------------------

        createInfo.enabledExtensionCount = glfwExtensionCount;  
        createInfo.ppEnabledExtensionNames = glfwExtensions;  

        createInfo.enabledLayerCount = 0;  

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {  
            throw std::runtime_error("failed to create Vulkan instance!");  
        }  

		std::cout << "Vulkan instance created successfully!" << std::endl;
    }

    void initWindow() 
    {
		glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Triangulo", nullptr, nullptr);
    }

    void initVulkan()
    {
        createInstance();
		pickPhysicalDevice();
    }

    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
		std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        std::cout << "Found device: " << deviceProperties.deviceName << std::endl;
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
			&& supportedFeatures.geometryShader;  // Check if the device supports geometry shaders
	}

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
		}
    }

    void cleanup()
    {
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
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