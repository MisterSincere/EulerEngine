/////////////////////////////////////////////////////////////////////
// Filename: window.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "windowClass.h"

#include "vulkanTools.h"

#include <cassert>

using namespace EE;


void glfw_onResize(GLFWwindow* window, int width, int height)
{
	if (width < 0 || height < 0) return;
	Window::UserData* ud = reinterpret_cast<Window::UserData*>(glfwGetWindowUserPointer(window));

	ud->window->settings.clientSize = { uint32_t(width), uint32_t(height) };

	if (ud->resizeCallback) ud->resizeCallback(window, width, height, ud->pUserData);
	else EE_PRINT("RESIZE: No callback function defined!\n");
}

void glfw_keyEvent(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
	Window* eewindow = reinterpret_cast<Window::UserData*>(glfwGetWindowUserPointer(window))->window;

	if (action == GLFW_PRESS) {
		eewindow->input.keysPressed[key] = true;
		eewindow->input.keysHit[key] = true;
	} else if (action == GLFW_RELEASE) {
		eewindow->input.keysPressed[key] = false;
	}
}

void glfw_cursorPos(GLFWwindow* window, double xpos, double ypos) {
	Window* eewindow = reinterpret_cast<Window::UserData*>(glfwGetWindowUserPointer(window))->window;

	eewindow->input.mouseXDelta = xpos - eewindow->input.mouseX;
	eewindow->input.mouseYDelta = ypos - eewindow->input.mouseY;
	eewindow->input.mouseX = xpos;
	eewindow->input.mouseY = ypos;
}


Window::Window()
{
	// Call glfw's init function and hint that we are going to use vulkan
	if (!glfwInit()) {
		EE_PRINT("GLFW Error: failed to initialize GLFW (internal error)!");
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //< Current convention for vulkan use

	// Initialize required instance extensions
	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0u; i < glfwExtensionCount; i++) {
		requiredInstanceExtensions.push_back(glfwExtensions[i]);
	}

	// Initialize required device extensions
	requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// Initialize the input values
	input.keysPressed = new bool[GLFW_KEY_LAST];
	input.keysHit = new bool[GLFW_KEY_LAST];
	assert(input.keysPressed && input.keysHit);
	memset(input.keysPressed, 0, sizeof(bool) * GLFW_KEY_LAST);
	memset(input.keysHit, 0, sizeof(bool) * GLFW_KEY_LAST);
}

Window::~Window()
{
	// Free memory
	RELEASE_A(input.keysPressed);
	RELEASE_A(input.keysHit);

	glfwDestroyWindow(window);

	glfwTerminate();
}

EEBool32 Window::Create(EEApplicationCreateInfo const& windowCInfo, EE::fpEEWindowResize resizeMethod, void* pUserData)
{
	// Check if the required instance extensions are available
	// required device extensions will be a criterion for the picked physical device
	uint32_t count;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	VkExtensionProperties* availableExtensions = new VkExtensionProperties[count];
	vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions);
	bool found{ false };  //< Indicates wether the current required extension was found
	for (char const* const reqExtension : requiredInstanceExtensions) {
		found = false;
		for (uint32_t i = 0u; i < count; i++) {
			if (strcmp(reqExtension, availableExtensions[i].extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			EE_PRINT("Failed to create the window: you do not support the required extension %s !\n", reqExtension);
			tools::exitFatal("Failed to create the window. Required extensions not supported!");
		}
	}
	delete[] availableExtensions;

	// Init window will setup the window according to the create info
	if (!glfw_createWindow(windowCInfo)) return false;

	// Set window user pointer
	userData.pUserData = pUserData;
	userData.resizeCallback = resizeMethod;
	userData.window = this;
	glfwSetWindowUserPointer(window, &userData);

	// Set up some of our callbacks
	glfwSetWindowSizeCallback(window, glfw_onResize);
	glfwSetKeyCallback(window, glfw_keyEvent);
	glfwSetCursorPosCallback(window, glfw_cursorPos);

	return EE_TRUE;
}

void Window::CreateSurface(VkInstance instance, VkAllocationCallbacks const * pAllocator)
{
	assert(instance != VK_NULL_HANDLE);
	VK_CHECK(glfwCreateWindowSurface(instance, this->window, pAllocator, &surface));
}

void Window::ReleaseSurface(VkInstance instance, VkAllocationCallbacks const* pAllocator) const
{
	assert(instance != VK_NULL_HANDLE);
	if (surface) vkDestroySurfaceKHR(instance, surface, pAllocator);
}

bool Window::PollEvents()
{
	// Reset input values
	memset(input.keysHit, 0, sizeof(bool) * GLFW_KEY_LAST);
	input.mouseXDelta = input.mouseYDelta = 0;

	glfwPollEvents();

	return glfwWindowShouldClose(window);
}

vulkan::SurfaceDetails Window::GetSurfaceDetails(VkPhysicalDevice physicalDevice) const
{
	vulkan::SurfaceDetails details;
	uint32_t count{ 0u };

	// Get the capabilities
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities));

	// Get the available surface formats
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr));
	details.formats.resize(count);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, details.formats.data()));

	// Get the available present modes to this surface
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr));
	details.presentModes.resize(count);
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, details.presentModes.data()));

	return details;
}

bool Window::IsAdequate(VkPhysicalDevice physicalDevice) const
{
	if (surface == VK_NULL_HANDLE) {
		EE_PRINT("Call CreateSurface to create a surface that can be checked against!\n");
		throw std::runtime_error("Call CreateSurface to create a surface that can be check against!\n");
	}
	
	// Extensions
	bool extensionSupport{ false };
	{
		// Get the supported extensions
		uint32_t count;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
		if (!count) return false;
		VkExtensionProperties* extensions = new VkExtensionProperties[count];
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions);

		// Check if the required device extensions are supported
		for (char const* curReqExtension : requiredDeviceExtensions) {
			extensionSupport = false;
			for (uint32_t i = 0u; i < count; i++) {
				// We found the extension so break here and set extensionSupport to true
				if (strcmp(curReqExtension, extensions[i].extensionName) == 0) {
					extensionSupport = true;
					break;
				}
			}
			// We checked all available extensions and did not find the required one
			if (!extensionSupport) break;
		}
	}

	// Surface support
	bool surfaceSupport{ false };
	{
		if (extensionSupport) {
			vulkan::SurfaceDetails details = GetSurfaceDetails(physicalDevice);
			surfaceSupport = details.formats.size() && details.presentModes.size();
		}
	}

	return extensionSupport && surfaceSupport;
}

bool Window::glfw_createWindow(EEApplicationCreateInfo const& cinfo)
{
	// Store settings that are not going to be changed (not clientSize and position)
	settings.title = cinfo.title;
	settings.screenMode = cinfo.screenMode;
	settings.mouseDisabled = cinfo.mouseDisabled;

	// Get information about the primary monitor
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	// Fake fullscreen mode (needs to be done says the doc)
	if (settings.screenMode == EE_SCREEN_MODE_FAKE_FULLSCREEN) {
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	}

	// Set client size
	if (settings.screenMode == EE_SCREEN_MODE_WINDOWED) {
		settings.clientSize = cinfo.clientSize;
	} else {
		settings.clientSize = { uint32_t(mode->width), uint32_t(mode->height) };
	}

	// Hint glfw to be maximized if maximized is desired
	if (settings.screenMode == EE_SCREEN_MODE_MAXIMIZED) {
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	} else {
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
	}

	// Create the window 
	monitor = (settings.screenMode & (EE_SCREEN_MODE_FULLSCREEN | EE_SCREEN_MODE_FAKE_FULLSCREEN)) ? glfwGetPrimaryMonitor() : nullptr;
	window = glfwCreateWindow(settings.clientSize.width, settings.clientSize.height, settings.title, monitor, nullptr);
	if (!window) {
		EE_PRINT("GLFW ERROR: Failed to create glfw window!\n");
		return false;
	}

	// Set position to custom position, but only when in windowed mode otherwise default to zero
	if (settings.screenMode == EE_SCREEN_MODE_WINDOWED) {
		settings.position = cinfo.position;
		if (cinfo.flags & EE_WINDOW_FLAGS_CENTERX) {
			settings.position.x = int32_t((mode->width - settings.clientSize.width) / 2);
		}
		if (cinfo.flags & EE_WINDOW_FLAGS_CENTERY) {
			settings.position.y = int32_t((mode->height - settings.clientSize.height) / 2);
		}
	} else {
		settings.position = { 0,0 };
	}

	// Icon setup
	if (cinfo.icon) {
		// @TODO
	}

	// Disable cursor if desired according to setting
	if (settings.mouseDisabled) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	return true;
}