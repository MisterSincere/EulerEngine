/////////////////////////////////////////////////////////////////////
// Filename: vulkanDevice.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDevice.h"

#include <cassert>

using namespace EE;



vulkan::Device::Device(EE::vulkan::Instance* pInstance, EE::Window* pWindow, VkAllocationCallbacks const* pAllocator)
{
	// instance and window need to be no nullptr
	assert(pInstance && pWindow);

	this->pWindow = pWindow;
	this->pInstance = pInstance;
	this->pAllocator = pAllocator;

	// Create a surface from the window
	pWindow->CreateSurface(*pInstance, pAllocator);

	// Store picked physical device
	physicalDevice = PickPhysicalDevice(*pInstance);

	// Store features, limits and properties of the picked physical device for later use
	// Device properties also contain limits and sparse properties
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	// Store supported device features
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
	// Memory properties that can be used for creating all kinds of buffers
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	// Get queue family properties
	uint32_t count{ 0u };
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
	assert(count > 0u);
	queueFamilyProperties.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());

	count = 0u;

	// LAYERS
	{
		// Store supported layers;
		vkEnumerateDeviceLayerProperties(physicalDevice, &count, nullptr);
		supportedLayers.resize(count);
		VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &count, supportedLayers.data()));
	}
	// EXTENSIONS
	{
		// Store supported extensions
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
		supportedExtensions.resize(count);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, supportedExtensions.data());
	}
}

vulkan::Device::~Device()
{
	if (cmdPoolGraphics) {
		vkDestroyCommandPool(logicalDevice, cmdPoolGraphics, pAllocator);
	}
	if (logicalDevice) {
		vkDestroyDevice(logicalDevice, pAllocator);
	}
	if (pWindow) {
		pWindow->ReleaseSurface(*pInstance, pAllocator);
	}
}

VkResult vulkan::Device::Create(VkPhysicalDeviceFeatures const& desiredFeatures, std::vector<char const*> const & additionalLayers, std::vector<char const*> const & additionalExtensions, VkQueueFlags requestedQueueTypes)
{
	// Queues that will be desired later need to be requested upon logical device creation
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	// Get the queue family indices for the requested family types
	// @note Note that the indices for different families may be the same

	float const defaultQueuePriority{ 0.0f };

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
		queueIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		VkDeviceQueueCreateInfo queueCInfo;
		queueCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCInfo.pNext = nullptr;
		queueCInfo.flags = 0;
		queueCInfo.queueFamilyIndex = queueIndices.graphics;
		queueCInfo.queueCount = 1u;
		queueCInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueCInfo);
	} else {
		queueIndices.graphics = 0u;
	}
	queueIndices.graphicsCount = queueFamilyProperties[queueIndices.graphics].queueCount;

	// Dedicated compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
		queueIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
		// We need to only create another queue if it is a different
		if (queueIndices.compute != queueIndices.graphics) {
			VkDeviceQueueCreateInfo queueCInfo;
			queueCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCInfo.pNext = nullptr;
			queueCInfo.flags = 0;
			queueCInfo.queueFamilyIndex = queueIndices.compute;
			queueCInfo.queueCount = 1u;
			queueCInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueCInfo);
		}
	} else {
		queueIndices.compute = queueIndices.graphics;
	}
	queueIndices.computeCount = queueFamilyProperties[queueIndices.compute].queueCount;

	// Present queue
	// @note So far there is no dedicated present queue, but it will be most definitely
	//			 a graphics queue
	{
		VkBool32 presentSupported{ VK_FALSE };
		for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, uint32_t(i), pWindow->surface, &presentSupported);
			if (presentSupported) {
				queueIndices.present = uint32_t(i);
				queueIndices.presentCount = queueFamilyProperties[i].queueCount;
				break;
			}
		}
		if (!presentSupported) {
			EE_PRINT("No queue for presenting detected!\n");
			EE::tools::exitFatal("No queue for presenting detected!\n");
		}
	}

	// Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
		queueIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
		// If the transfer queue is a different one than graphics or compute
		// also set up a create info for this one
		if (queueIndices.transfer != queueIndices.graphics
				&& queueIndices.transfer != queueIndices.compute) {
			VkDeviceQueueCreateInfo queueCInfo;
			queueCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCInfo.pNext = nullptr;
			queueCInfo.flags = 0;
			queueCInfo.queueFamilyIndex = queueIndices.transfer;
			queueCInfo.queueCount = 1u;
			queueCInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueCInfo);
		}
	} else {
		queueIndices.transfer = queueIndices.graphics;
	}
	queueIndices.transferCount = queueFamilyProperties[queueIndices.transfer].queueCount;


	// Check for each additional layer if it is supported, if so store it
	for (char const* curAdditionalLayer : additionalLayers) {
		if (LayerSupported(curAdditionalLayer)) {
			enabledLayers.push_back(curAdditionalLayer);
		} else {
			EE_PRINT("[DEVICE] Layer %s was requested but is not supported!\n", curAdditionalLayer);
		}
	}

	// Check for each additional extension if it is supported, if so store it
	{
		// Store the extensions required for the window,
		// which were already checked in the Window::IsAdequate method
		enabledExtensions.insert(enabledExtensions.end(),
			pWindow->requiredDeviceExtensions.begin(),
			pWindow->requiredDeviceExtensions.end());

		// Now add supported additional extensions
		for (char const* curAdditionalExtension : additionalExtensions) {
			if (ExtensionSupported(curAdditionalExtension)) {
				enabledExtensions.push_back(curAdditionalExtension);
			} else {
				EE_PRINT("[DEVICE] Extension %s was requested but is not supported!\n", curAdditionalExtension);
			}
		}
	}

	// Check feature support
	{
		// Looks ugly but is valid since we know that the whole struct contains only VkBool32
		VkBool32* iSupportedFeature = reinterpret_cast<VkBool32*>(&supportedFeatures);
		VkBool32* iDesiredFeature = reinterpret_cast<VkBool32*>(const_cast<VkPhysicalDeviceFeatures*>(&desiredFeatures));
		VkBool32* iEnabledFeature = reinterpret_cast<VkBool32*>(&enabledFeatures);
		uint32_t amountFeaturesToCheck = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
		for (uint32_t i = 0u; i < amountFeaturesToCheck; i++) {
			// First we just set enabled to the desired bool/value
			*iEnabledFeature = *iDesiredFeature;
			
			// !desiredFeature || supportedFeature <=> desiredFeature => supportedFeature
			// We want the opposite: a true value if it is not supported but desired, since this is
			// what we wanna catch, so: desiredFeature && !supportedFeature
			if ((*iDesiredFeature) && !(*iSupportedFeature)) {
				// Feature is not supported so we need to set it to false
				*iEnabledFeature = VK_FALSE;
				// Print message
				EE_PRINT("[DEVICE] %d. Feature requested but not supported!\n", i + 1);
				EE::tools::warning("Not all device requested device features are supported!\n");
			}

			// Increase iterators
			iSupportedFeature++;
			iDesiredFeature++;
			iEnabledFeature++;
		}
	}

	// Finally create the logical device representation
	VkDeviceCreateInfo deviceCInfo;
	deviceCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCInfo.pNext = nullptr;
	deviceCInfo.flags = 0;
	deviceCInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
	deviceCInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCInfo.enabledLayerCount = uint32_t(enabledLayers.size());
	deviceCInfo.ppEnabledLayerNames = enabledLayers.data();
	deviceCInfo.enabledExtensionCount = uint32_t(enabledExtensions.size());
	deviceCInfo.ppEnabledExtensionNames = enabledExtensions.data();
	deviceCInfo.pEnabledFeatures = &enabledFeatures;

	VkResult result = vkCreateDevice(physicalDevice, &deviceCInfo, pAllocator, &logicalDevice);

	if (result == VK_SUCCESS) {
		// Create the default command pool for drawing
		cmdPoolGraphics = CreateCommandPool(queueIndices.graphics);
	}

	return VkResult();
}

vulkan::ExecBuffer vulkan::Device::CreateCommandBuffer(VkCommandBufferLevel level, bool begin, bool singleTime)
{
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = initializers::commandBufferAllocateInfo(cmdPoolGraphics, level, 1u);

	ExecBuffer execBuffer;
	VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &cmdBufferAllocInfo, &execBuffer.cmdBuffer));

	// Open the buffer for recording if requested
	if (begin) {
		VkCommandBufferBeginInfo beginInfo = initializers::commandBufferBeginInfo((singleTime)
																					 ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);
		VK_CHECK(vkBeginCommandBuffer(execBuffer.cmdBuffer, &beginInfo));
	}

	execBuffer.queue = AcquireQueue(GRAPHICS_FAMILY);
	execBuffer.device = this;

	return execBuffer;
}

VkResult vulkan::Device::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties,
	VkDeviceSize size, VkBuffer* pBufferOut, VkDeviceMemory* pBufferMemoryOut, void* pData) const
{
	// Create the buffer handle
	VkBufferCreateInfo bufferCInfo = vulkan::initializers::bufferCreateInfo(usageFlags, size);
	VK_CHECK(vkCreateBuffer(logicalDevice, &bufferCInfo, pAllocator, pBufferOut));

	// Allocate the memory for the buffer
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(logicalDevice, *pBufferOut, &memReqs);
	VkMemoryAllocateInfo allocInfo = vulkan::initializers::memoryAllocateInfo();
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryProperties);
	VK_CHECK(vkAllocateMemory(logicalDevice, &allocInfo, pAllocator, pBufferMemoryOut));

	// If data has been passed in store this data in the buffer
	if (pData) {
		void* mapped{ nullptr };
		VK_CHECK(vkMapMemory(logicalDevice, *pBufferMemoryOut, 0, size, 0, &mapped));
		memcpy(mapped, pData, size);
		// If host coherency hasnt been requested to a manually flush to make writes visible
		if ((memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			VkMappedMemoryRange mappedRange = vulkan::initializers::mappedMemoryRange(*pBufferMemoryOut, size);
			vkFlushMappedMemoryRanges(logicalDevice, 1u, &mappedRange);
		}
		vkUnmapMemory(logicalDevice, *pBufferMemoryOut);
	}

	// Attach the memory to the buffer
	VK_CHECK(vkBindBufferMemory(logicalDevice, *pBufferOut, *pBufferMemoryOut, 0));

	return VK_SUCCESS;
}

VkCommandPool vulkan::Device::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo cmdPoolCInfo;
	cmdPoolCInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCInfo.pNext = nullptr;
	cmdPoolCInfo.flags = createFlags;
	cmdPoolCInfo.queueFamilyIndex = queueFamilyIndex;
	VkCommandPool cmdPool;
	VK_CHECK(vkCreateCommandPool(logicalDevice, &cmdPoolCInfo, pAllocator, &cmdPool));
	return cmdPool;
}

VkQueue vulkan::Device::AcquireQueue(QueueTypeFlagBits requestedFamily)
{
	uint32_t index;
	if (requestedFamily & GRAPHICS_FAMILY) {
		index = queueIndices.graphics;
	} else if (requestedFamily & COMPUTE_FAMILY) {
		index = queueIndices.compute;
	} else if (requestedFamily & TRANSFER_FAMILY) {
		index = queueIndices.transfer;
	} else if (requestedFamily & PRESENT_FAMILY) {
		index = queueIndices.present;
	} else {
		EE_PRINT("[DEVICE] Invalid queue type requested to acquire!\n");
		EE::tools::exitFatal("[DEVICE] Invalid queue type requested to acquire!\n");
	}

	VkQueue queue;
	vkGetDeviceQueue(logicalDevice, index, 0u, &queue);
	return queue;
}



uint32_t vulkan::Device::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
	// First check for a queue family that matches the desired type exactly
	for (uint32_t i = 0u; i < uint32_t(queueFamilyProperties.size()); i++) {
		if ((queueFamilyProperties[i].queueFlags | queueFlags) == queueFlags) {
			return i;
		}
	}

	// No exact match found so check for a queue family that does at least
	// the required things
	for (uint32_t i = 0u; i < uint32_t(queueFamilyProperties.size()); i++) {
		if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
			return i;
		}
	}

	EE_PRINT("[DEVICE] Could not find a matching queue family index. Asked flag %d\n", queueFlags);
	throw std::runtime_error("Could not find a matching queue family index!\n");
}

uint32_t vulkan::Device::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
	for (uint32_t i = 0u; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				if (memTypeFound) *memTypeFound = VK_TRUE;
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound) {
		*memTypeFound = VK_FALSE;
		return 0u;
	} else {
		EE_PRINT("[DEVICE] Could not find matching memory type!\n");
		throw std::runtime_error("Could not find matching memory type!\n");
	}
}


bool vulkan::Device::IsFormatSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);;

	if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags) {
		return true;
	}
	if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags) {
		return true;
	}

	return false;
}

bool vulkan::Device::LayerSupported(char const* layer)
{
	for (size_t i = 0; i < supportedLayers.size(); i++) {
		if (strcmp(supportedLayers[i].layerName, layer) == 0) {
			return true;
		}
	}
	return false;
}

bool vulkan::Device::ExtensionSupported(char const* extension)
{
	for (size_t i = 0; i < supportedExtensions.size(); i++) {
		if (strcmp(supportedExtensions[i].extensionName, extension) == 0) {
			return true;
		}
	}
	return false;
}

VkPhysicalDevice vulkan::Device::PickPhysicalDevice(VkInstance instance)
{
	assert(instance != VK_NULL_HANDLE);

	uint32_t deviceCount{ 0u };
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Check if at least on gpu is available
	if (!deviceCount) {
		EE_PRINT("No GPU found!\n");
		EE::tools::exitFatal("No GPU found on your system!");
	}

	// Allocate memory that will hold the physical devices of this system
	VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

	VkPhysicalDevice pickedDevice{ VK_NULL_HANDLE };
	for (uint32_t i = 0u; i < deviceCount; i++) {
		if (pWindow->IsAdequate(physicalDevices[i])) {
			pickedDevice = physicalDevices[i];
			break;
		}
	}

	// If the picked device handle is still nullptr, just take the first one and print warning
	if (pickedDevice == VK_NULL_HANDLE) {
		pickedDevice = physicalDevices[0];
		EE_PRINT("No suitable GPU found!\n");
		EE::tools::warning("No suitable GPU found! First one was chosen but could crash!\n");
	}

	delete[] physicalDevices;
	return pickedDevice;
}


void vulkan::ExecBuffer::Execute(VkSubmitInfo* _submitInfo, bool wait, bool free)
{
	assert(cmdBuffer);

	VK_CHECK(vkEndCommandBuffer(cmdBuffer));

	// Submit info (passed in will be used if not nullptr)
	VkSubmitInfo submitInfo;
	if (_submitInfo) submitInfo = *_submitInfo;
	else submitInfo = initializers::submitInfo(&cmdBuffer, 1u);

	// Create fence if we wanna wait
	VkFence fence{ VK_NULL_HANDLE };
	if (wait)
	{
		VkFenceCreateInfo fenceCInfo = initializers::fenceCreateInfo(VK_FLAGS_NONE);
		VK_CHECK(vkCreateFence(*device, &fenceCInfo, device->pAllocator, &fence));
	}

	// Submit to the queue
	vkQueueSubmit(queue, 1u, &submitInfo, fence);

	// Wait for the fence to signal that the cmd buffer has finished execution and destroy afterwards (if desired)
	if (fence != VK_NULL_HANDLE)
	{
		VK_CHECK(vkWaitForFences(*device, 1u, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		vkDestroyFence(*device, fence, device->pAllocator);
	}

	if (free)
	{
		vkFreeCommandBuffers(*device, device->cmdPoolGraphics, 1u, &cmdBuffer);
	}
}

