/////////////////////////////////////////////////////////////////////
// Filename: vulkanInstance.cp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanInstance.h"


EE::vulkan::Instance::Instance(EE::Window const& window, std::vector<char const*> additionalLayers, std::vector<char const*> additionalExtensions)
{
	uint32_t count;

	// LAYERS
	{
		// Store supported layers
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		supportedLayers.resize(count);
		vkEnumerateInstanceLayerProperties(&count, supportedLayers.data());

		// Store the desired layers that are also supported as enabled
		bool supported{ false };
		for (char const* const& curDesiredLayer : additionalLayers) {
			supported = false;
			for (VkLayerProperties const& curSupportedLayer : supportedLayers) {
				// If the names are the same the layer is supported so we can stop searching
				if (strcmp(curDesiredLayer, curSupportedLayer.layerName) == 0) {
					supported = true;
					break;
				}
			}

			// Push back desired layer if it is supported otherwise show notification
			if (supported) enabledLayers.push_back(curDesiredLayer);
			else EE_PRINT("[VulkanInstance] %s layer was requested but is not supported!\n", curDesiredLayer);
		}
	}

	// EXTENSIONS
	{
		// Store supported extensions
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		supportedExtensions.resize(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, supportedExtensions.data());

		// Store the desired extensions that are also supported as enabled
		bool supported{ false };
		for (char const* const& curDesiredExtension : additionalExtensions) {
			supported = false;
			for (VkExtensionProperties const& curSupportedExtension : supportedExtensions) {
				// If the names are the same the extension is supported so we can stop searching
				if (strcmp(curDesiredExtension, curSupportedExtension.extensionName) == 0) {
					supported = true;
					break;
				}
			}

			// Push back desired extension if it is supported otherwise show notification
			if (supported) enabledExtensions.push_back(curDesiredExtension);
			else EE_PRINT("[VulkanInstance] %s extension was requested but is not supported!\n", curDesiredExtension);
		}

		// Add required extensions from the window (checked at window creation)
		enabledExtensions.insert(enabledExtensions.end(), window.requiredInstanceExtensions.begin(), window.requiredInstanceExtensions.end());
	}
}

EE::vulkan::Instance::~Instance()
{
	if (instance) vkDestroyInstance(instance, pAllocator);

	// Free memory
	supportedLayers.~vector();
	enabledLayers.~vector();
	supportedExtensions.~vector();
	enabledExtensions.~vector();
}

VkResult EE::vulkan::Instance::Create(VkAllocationCallbacks const* pAllocator)
{
	// Store the allocator for future use
	this->pAllocator = pAllocator;

	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = name;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = name;
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = apiVersion;

	VkInstanceCreateInfo instanceCInfo;
	instanceCInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCInfo.pNext = nullptr;
	instanceCInfo.flags = 0;
	instanceCInfo.pApplicationInfo = &appInfo;
	instanceCInfo.enabledLayerCount = uint32_t(enabledLayers.size());
	instanceCInfo.ppEnabledLayerNames = enabledLayers.data();
	instanceCInfo.enabledExtensionCount = uint32_t(enabledExtensions.size());
	instanceCInfo.ppEnabledExtensionNames = enabledExtensions.data();

	return vkCreateInstance(&instanceCInfo, pAllocator, &instance);
}
