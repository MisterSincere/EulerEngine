/////////////////////////////////////////////////////////////////////
// Filename: vulkanSwapchain.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanSwapchain.h"

#include <cassert>
#include <algorithm>

using namespace EE;


// Foreward declaration of helper functions that are needed to get supported settings
VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
VkPresentModeKHR ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);
VkExtent2D ChooseSwapchainExtent(VkSurfaceCapabilitiesKHR const& surfaceCapabilities, GLFWwindow*);
VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha(VkSurfaceCapabilitiesKHR const& surfaceCapabilities);


vulkan::Swapchain::Swapchain(Device const* pDevice, Window const* pWindow, bool vsync)
	: pDevice(pDevice)
	, pWindow(pWindow)
{
	assert(pDevice && pWindow);

	// Store the surface details since they wont/shouldnt change from now on
	surfaceDetails = pWindow->GetSurfaceDetails(pDevice->physicalDevice);

	// SETTINGS
	{
		settings.surfaceFormat = ChooseSwapchainSurfaceFormat(surfaceDetails.formats);

		settings.presentMode = (vsync)
														? VK_PRESENT_MODE_FIFO_KHR
														: ChooseSwapchainPresentMode(surfaceDetails.presentModes);

		settings.extent = ChooseSwapchainExtent(surfaceDetails.capabilities, pWindow->window);

		settings.compositeAlpha = ChooseCompositeAlpha(surfaceDetails.capabilities);

		settings.preTransform = (surfaceDetails.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
														 ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
														 : surfaceDetails.capabilities.currentTransform;

		settings.vsyncEnabled = vsync;

		settings.imageCount = surfaceDetails.capabilities.minImageCount + 1;
		if (surfaceDetails.capabilities.maxImageCount > 0
				&& settings.imageCount > surfaceDetails.capabilities.maxImageCount)
		{
			settings.imageCount = surfaceDetails.capabilities.maxImageCount;
		}
	}
	
	// FUNCTION POINTERS
	fpCreateSwapchainKHR = VK_FUNCTION(*(pDevice->pInstance), vkCreateSwapchainKHR);
	fpDestroySwapchainKHR = VK_FUNCTION(*(pDevice->pInstance), vkDestroySwapchainKHR);
	fpGetSwapchainImagesKHR = VK_FUNCTION(*(pDevice->pInstance), vkGetSwapchainImagesKHR);
	fpAcquireNextImageKHR = VK_FUNCTION(*(pDevice->pInstance), vkAcquireNextImageKHR);
	fpQueuePresentKHR = VK_FUNCTION(*(pDevice->pInstance), vkQueuePresentKHR);
}

vulkan::Swapchain::~Swapchain()
{
	for (size_t i = 0; i < buffers.size(); i++) {
		vkDestroyImageView(*pDevice, buffers[i].imageView, pDevice->pAllocator);
	}

	if (swapchain != VK_NULL_HANDLE) {
		fpDestroySwapchainKHR(*pDevice, swapchain, pDevice->pAllocator);
		swapchain = VK_NULL_HANDLE;
	}
}

void vulkan::Swapchain::Create()
{
	assert(fpCreateSwapchainKHR && fpDestroySwapchainKHR && fpGetSwapchainImagesKHR);

	// If the swapchain was already created we wanna recreate the swapchain with updated settings
	if (isCreated) {
		// Update surface details
		surfaceDetails = pWindow->GetSurfaceDetails(pDevice->physicalDevice);

		// SETTINGS
		settings.surfaceFormat = ChooseSwapchainSurfaceFormat(surfaceDetails.formats);

		settings.presentMode = (settings.vsyncEnabled)
														? VK_PRESENT_MODE_FIFO_KHR
														: ChooseSwapchainPresentMode(surfaceDetails.presentModes);

		settings.extent = ChooseSwapchainExtent(surfaceDetails.capabilities, pWindow->window);

		settings.compositeAlpha = ChooseCompositeAlpha(surfaceDetails.capabilities);

		settings.preTransform = (surfaceDetails.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
														 ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
														 : surfaceDetails.capabilities.currentTransform;

		settings.imageCount = surfaceDetails.capabilities.minImageCount + 1;
		if (surfaceDetails.capabilities.maxImageCount > 0
				&& settings.imageCount > surfaceDetails.capabilities.maxImageCount)
		{
			settings.imageCount = surfaceDetails.capabilities.maxImageCount;
		}
	}

	VkSwapchainKHR oldSwapchain = swapchain; //< Will be null if this is the first creation

	VkSwapchainCreateInfoKHR swapchainCInfo;
	swapchainCInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCInfo.pNext = nullptr;
	swapchainCInfo.flags = 0;
	swapchainCInfo.surface = pWindow->surface;
	swapchainCInfo.minImageCount = settings.imageCount;
	swapchainCInfo.imageFormat = settings.surfaceFormat.format;
	swapchainCInfo.imageColorSpace = settings.surfaceFormat.colorSpace;
	swapchainCInfo.imageExtent = settings.extent;
	swapchainCInfo.imageArrayLayers = 1;
	swapchainCInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCInfo.queueFamilyIndexCount = 0u;
	swapchainCInfo.pQueueFamilyIndices = nullptr;
	swapchainCInfo.preTransform = settings.preTransform;
	swapchainCInfo.compositeAlpha = settings.compositeAlpha;
	swapchainCInfo.presentMode = settings.presentMode;
	swapchainCInfo.clipped = VK_TRUE; //< Discards rendering outside of surface
	swapchainCInfo.oldSwapchain = oldSwapchain;

	// Set additional usage flags for blitting from the swapchain images if supported
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(pDevice->physicalDevice, settings.surfaceFormat.format, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT
			|| formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)
	{
		swapchainCInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	VK_CHECK(fpCreateSwapchainKHR(*pDevice, &swapchainCInfo, pDevice->pAllocator, &swapchain));

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE) {
		for (uint32_t i = 0u; i < settings.imageCount; i++) {
			vkDestroyImageView(*pDevice, buffers[i].imageView, pDevice->pAllocator);
		}
		fpDestroySwapchainKHR(*pDevice, oldSwapchain, pDevice->pAllocator);
	}
	VK_CHECK(fpGetSwapchainImagesKHR(*pDevice, swapchain, &settings.imageCount, nullptr));

	// Get the swapchain images
	images.resize(settings.imageCount);
	VK_CHECK(fpGetSwapchainImagesKHR(*pDevice, swapchain, &settings.imageCount, images.data()));

	// Store the swapchain buffers alias images and their imageViews
	buffers.resize(settings.imageCount);
	for (uint32_t i = 0u; i < settings.imageCount; i++) {
		buffers[i].image = images[i];

		VkImageViewCreateInfo imageViewCInfo;
		imageViewCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCInfo.pNext = nullptr;
		imageViewCInfo.flags = 0;
		imageViewCInfo.image = buffers[i].image;
		imageViewCInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCInfo.format = settings.surfaceFormat.format;
		imageViewCInfo.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		imageViewCInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCInfo.subresourceRange.baseMipLevel = 0u;
		imageViewCInfo.subresourceRange.levelCount = 1u;
		imageViewCInfo.subresourceRange.baseArrayLayer = 0u;
		imageViewCInfo.subresourceRange.layerCount = 1u;

		VK_CHECK(vkCreateImageView(*pDevice, &imageViewCInfo, pDevice->pAllocator, &buffers[i].imageView));
	}

	// Swapchain was successfully created/recreated
	isCreated = true;
}

VkResult vulkan::Swapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const
{
	assert(fpAcquireNextImageKHR);

	if (!isCreated) return VK_ERROR_INITIALIZATION_FAILED;
	return fpAcquireNextImageKHR(*pDevice, swapchain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
}

VkResult vulkan::Swapchain::PresentImage(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) const
{
	assert(fpQueuePresentKHR);

	if (!isCreated) return VK_ERROR_INITIALIZATION_FAILED;

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1u;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	// Set semaphore to wait for if one was passed in
	if (waitSemaphore != VK_NULL_HANDLE) {
		presentInfo.waitSemaphoreCount = 1u;
		presentInfo.pWaitSemaphores = &waitSemaphore;
	} else {
		presentInfo.waitSemaphoreCount = 0u;
		presentInfo.pWaitSemaphores = nullptr;
	}

	return fpQueuePresentKHR(queue, &presentInfo);
}





VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats)
{
	// This means we can choose totally freely
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (VkSurfaceFormatKHR const& curFormat : availableFormats) {
		if (curFormat.format == VK_FORMAT_B8G8R8A8_UNORM && curFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return curFormat;
	}

	//@TODO:: Currently we just check for one and then immediately go "fuck it" and return the first one
	//				Maybe go stepwise down within a preferred list
	return availableFormats[0];
}

VkPresentModeKHR ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes)
{
	// Holds the currently best present mode supported
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (VkPresentModeKHR const& curPresentMode : availablePresentModes) {
		if (curPresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return curPresentMode; //< Best one we can wish for
		} else if (curPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = curPresentMode;
		}
	}

	return bestMode;
}

VkExtent2D ChooseSwapchainExtent(VkSurfaceCapabilitiesKHR const& surfaceCapabilities, GLFWwindow* pWindow)
{
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return surfaceCapabilities.currentExtent;
	}

	int width, height;
	glfwGetWindowSize(pWindow, &width, &height);
	VkExtent2D actualExtent = { uint32_t(width), uint32_t(height) };

	// Clamp possible bad values from glfw to the actual possible range of the surface
	actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
		std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
		std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha(VkSurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	// Preferred composite alpha flag
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Select the first one supported of the following list
	std::vector<VkCompositeAlphaFlagBitsKHR> priorityList = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for (VkCompositeAlphaFlagBitsKHR const& curAlpha : priorityList) {
		if (surfaceCapabilities.supportedCompositeAlpha & curAlpha) {
			compositeAlpha = curAlpha;
			break;
		}
	}

	return compositeAlpha;
}