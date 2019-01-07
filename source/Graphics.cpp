/////////////////////////////////////////////////////////////////////
// Filename: Graphics.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "Graphics.h"

#include <cassert>

#include "vkcore/vulkanShader.h"
#include "vkcore/vulkanMesh.h"
#include "vkcore/vulkanObject.h"
#include "vkcore/vulkanResources.h"

#define LAST_ELEMENT(vec) (vec[vec.size() - 1])

using namespace EE;
using namespace DirectX;


Graphics::Graphics()
{}

Graphics::~Graphics()
{
	pRenderer->WaitTillIdle();

	// OBJECTS
	for (size_t i = 0u; i < currentObjects.size(); i++) {
		delete currentObjects[i];
		delete iCurrentObjects[i];
	}
	currentObjects.~vector();
	iCurrentObjects.~vector();

	// TEXTURES
	for (size_t i = 0u; i < currentTextures.size(); i++) {
		delete currentTextures[i];
		delete iCurrentTextures[i];
	}
	currentTextures.~vector();
	iCurrentTextures.~vector();

	// SHADER
	for (size_t i = 0u; i < currentShader.size(); i++) {
		delete currentShader[i];
		delete iCurrentShader[i];
	}
	currentShader.~vector();
	iCurrentShader.~vector();

	// MESHES
	for (size_t i = 0u; i < currentMeshes.size(); i++) {
		delete currentMeshes[i];
		delete iCurrentMeshes[i];
	}
	currentMeshes.~vector();
	iCurrentMeshes.~vector();

	// BUFFERS
	for (size_t i = 0u; i < currentBuffers.size(); i++) {
		delete currentBuffers[i];
		delete iCurrentBuffers[i];
	}
	currentBuffers.~vector();
	iCurrentBuffers.~vector();

	// Destroy vulkan core instances
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

	// Initialize some drawing matrices
	XMStoreFloat4x4(&matrices.orthoLH, XMMatrixOrthographicLH(float(pSwapchain->settings.extent.width), float(pSwapchain->settings.extent.height), settings.nearPlane, settings.farPlane));
	XMStoreFloat4x4(&matrices.orthoRH, XMMatrixOrthographicRH(float(pSwapchain->settings.extent.width), float(pSwapchain->settings.extent.height), settings.nearPlane, settings.farPlane));

	XMStoreFloat4x4(&matrices.projLH, XMMatrixPerspectiveFovLH(XM_PIDIV2, float(pSwapchain->settings.extent.width) / float(pSwapchain->settings.extent.height), settings.nearPlane, settings.farPlane));
	XMStoreFloat4x4(&matrices.projRH, XMMatrixPerspectiveFovRH(XM_PIDIV2, float(pSwapchain->settings.extent.width) / float(pSwapchain->settings.extent.height), settings.nearPlane, settings.farPlane));

	// base view matrices
	XMVECTOR position = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&matrices.baseViewLH, XMMatrixLookAtLH(position, target, up));
	XMStoreFloat4x4(&matrices.baseViewRH, XMMatrixLookAtRH(position, target, up));

	return true;
}

void EE::Graphics::Draw()
{
	pRenderer->RecordDrawCommands(currentObjects);
	pRenderer->Draw();
}

void EE::Graphics::Resize()
{
	pRenderer->Resize(currentObjects);
}

EEMesh EE::Graphics::CreateMesh(void const* pVertices, size_t amountVertices, std::vector<uint32_t> const & indices)
{
	EE_INVARIANT(iCurrentMeshes.size() == currentMeshes.size());

	// Push back new mesh handle
	currentMeshes.push_back(new EE::Mesh(pRenderer));
	LAST_ELEMENT(currentMeshes)->Create(pVertices, amountVertices, indices);

	// Push back address of the index to the new mesh
	iCurrentMeshes.push_back(new uint32_t((uint32_t)currentMeshes.size() - 1u));

	EE_INVARIANT(iCurrentMeshes.size() == currentMeshes.size());

	return { LAST_ELEMENT(iCurrentShader) };
}

EEBuffer EE::Graphics::CreateBuffer(size_t bufferSize)
{
	EE_INVARIANT(iCurrentBuffers.size() == currentBuffers.size());

	// Push back new buffer handle
	currentBuffers.push_back(new EE::Buffer(pDevice, bufferSize));

	// Push back address of the index to the new buffer
	iCurrentBuffers.push_back(new uint32_t((uint32_t)currentBuffers.size() - 1u));

	EE_INVARIANT(iCurrentBuffers.size() == currentBuffers.size());

	return { LAST_ELEMENT(iCurrentBuffers) };
}

EETexture EE::Graphics::CreateTexture(char const* fileName, bool enableMipMapping, bool unnormalizedCoordinates)
{
	EE_INVARIANT(iCurrentTextures.size() == currentTextures.size());

	// Push back new texture handle
	currentTextures.push_back(new EE::Texture(pRenderer, fileName, enableMipMapping, unnormalizedCoordinates));
	LAST_ELEMENT(currentTextures)->Upload();

	// Push back address of the index to the new texture
	iCurrentTextures.push_back(new uint32_t((uint32_t)currentTextures.size() - 1));

	EE_INVARIANT(iCurrentTextures.size() == currentTextures.size());

	return EETexture();
}

EETexture EE::Graphics::CreateTexture(EETextureCreateInfo const & textureCInfo)
{
	EE_INVARIANT(iCurrentTextures.size() == currentTextures.size());

	// Push back new texture handle
	currentTextures.push_back(new EE::Texture(pRenderer, textureCInfo));
	LAST_ELEMENT(currentTextures)->Upload();

	// Push back address of the index to the new texture
	iCurrentTextures.push_back(new uint32_t((uint32_t)currentTextures.size() - 1));

	EE_INVARIANT(iCurrentTextures.size() == currentTextures.size());

	return EETexture();
}

EEShader Graphics::CreateShader(EEShaderCreateInfo const& cinfo)
{
	EE_INVARIANT(iCurrentShader.size() == currentShader.size());

	// Push back new shader handle
	currentShader.push_back(new EE::Shader(pRenderer, cinfo));
	if (!LAST_ELEMENT(currentShader)->Create()) {
		delete LAST_ELEMENT(currentShader);
		currentShader.erase(currentShader.end());
		EE_INVARIANT(iCurrentObjects.size() == currentObjects.size());
		return nullptr;
	}

	// Push back address of the index to the new shader
	iCurrentShader.push_back(new uint32_t((uint32_t)currentShader.size() - 1u));

	EE_INVARIANT(iCurrentShader.size() == currentShader.size());

	return { LAST_ELEMENT(iCurrentShader) };
}

EEObject EE::Graphics::CreateObject(EEShader shader, EEMesh mesh, std::vector<EEObjectResourceBinding> const & bindings, EESplitscreen splitscreen)
{
	EE_INVARIANT(iCurrentObjects.size() == currentObjects.size());

	// Push back new object handle
	currentObjects.push_back(new EE::Object(pRenderer, currentShader[*shader], currentMeshes[*mesh], splitscreen));
	if (!LAST_ELEMENT(currentObjects)->Create(bindings, currentTextures, currentBuffers)) {
		delete LAST_ELEMENT(currentObjects);
		currentObjects.erase(currentObjects.end());
		EE_INVARIANT(iCurrentObjects.size() == currentObjects.size());
		return nullptr;
	}

	// Push back address of the index to the new object
	iCurrentObjects.push_back(new uint32_t((uint32_t)currentObjects.size() - 1));

	EE_INVARIANT(iCurrentObjects.size() == currentObjects.size());

	return { LAST_ELEMENT(iCurrentObjects) };
}


void EE::Graphics::UpdateBuffer(EEBuffer buffer, void const * pData)
{
	if ((*buffer) < 0 || (*buffer) >= currentBuffers.size()) {
		EE_PRINT("[GRAPHICS] Invalid buffer handle passed in to be updated!\n");
		return;
	}
	currentBuffers[*buffer]->Update(pData);
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