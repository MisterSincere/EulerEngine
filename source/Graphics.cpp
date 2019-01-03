/////////////////////////////////////////////////////////////////////
// Filename: Graphics.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "Graphics.h"

#include <cassert>

using namespace EE;

Graphics::Graphics()
{

}

Graphics::~Graphics()
{
	pRenderer->WaitTillIdle();
	RELEASE_S(pRenderer);
	RELEASE_S(pSwapchain);
	RELEASE_S(pDevice);
	RELEASE_S(pDebug);
	RELEASE_S(pInstance);
}

bool Graphics::Create(Window* pWindow, EEApplicationCreateInfo const& info)
{
	// Store the pointer to the used window
	this->pWindow = pWindow;

	// Vulkan Instance
	vk_instance();

	// Setup debug handler if we want validation
	if (settings.validation) {
		vk_debug();
	}

	// Physical and logical device setup
	vk_device();

	// Swapchain setup
	vk_swapchain();

	// Create the renderer
	vk_renderer(info);


	return true;
}

void Graphics::vk_instance()
{
	// LAYERS
	std::vector<char const*> instanceLayers;
	{
		// If the validation is enabled the lunarg validation layer will be used
		if (settings.validation) {

			// The lunarg standard validation layer will load the following in the order listed:
			//	- VK_LAYER_GOOGLE_thrading
			//	- VK_LAYER_LUNARG_parameter_validation
			//	- VK_LAYER_LUNARG_device_limits
			//	- VK_LAYER_LUNARG_object_tracker
			//	- VK_LAYER_LUNARG_image
			//	- VK_LAYER_LUNARG_core_validation
			//	- VK_LAYER_LUNARG_swapchain
			//	- VK_LAYER_GOOGLE_swapchain
			instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");


			//instanceLayers.push_back("VK_LAYER_RENDERDOC_Capture");
		}
	}

	// EXTENSIONS
	std::vector<char const*> instanceExtensions;
	{
		// If the validation is enabled the debug report extension will be used
		if (settings.validation) {
			instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
	}

	pInstance = new vulkan::Instance(*pWindow, instanceLayers, instanceExtensions);
	assert(pInstance);
	VK_CHECK(pInstance->Create(pAllocator));
}

void Graphics::vk_debug()
{
	pDebug = new vulkan::Debug(pInstance);
	pDebug->Create();
}

void Graphics::vk_device()
{
	std::vector<char const*> extensions(0);
	std::vector<char const*> layers(0);

	VkPhysicalDeviceFeatures enabledFeatures{ 0 };
	enabledFeatures.samplerAnisotropy = VK_TRUE;
	enabledFeatures.fillModeNonSolid = VK_TRUE;

	// Create the device handle
	pDevice = new vulkan::Device(pInstance, pWindow, pAllocator);
	VK_CHECK(pDevice->Create(enabledFeatures, layers, extensions));
}

void Graphics::vk_swapchain()
{
	pSwapchain = new vulkan::Swapchain(pDevice, pWindow);
	pSwapchain->Create();
}

void Graphics::vk_renderer(EEApplicationCreateInfo const& info)
{
	pRenderer = new vulkan::Renderer(pSwapchain, info);
	if (info.rendererType & EE_RENDER_TYPE_3D) {
		pRenderer->Create3D();
	}
	if (info.rendererType & EE_RENDER_TYPE_2D) {
		pRenderer->Create2D();
	}
}