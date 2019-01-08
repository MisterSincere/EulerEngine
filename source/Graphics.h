/////////////////////////////////////////////////////////////////////
// Filename: Graphics.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vkcore/vulkanRenderer.h"


namespace EE {

	struct Shader;
	struct Mesh;
	struct Buffer;
	struct Texture;

	struct Graphics
	{
		/* @brief The window this graphics are created for */
		Window* pWindow{ nullptr };
		/* @brief The VkAllocationCallbacks currently in use (no one) */
		VkAllocationCallbacks const* pAllocator{ nullptr };
		/* @brief A struct that manages handling the vulkan instance */
		vulkan::Instance* pInstance{ nullptr };
		/* @brief Implementation of the debug report extension */
		vulkan::Debug* pDebug{ nullptr };
		/* @brief Struct that manages physical and logical device and command buffer allocation (queues) */
		vulkan::Device* pDevice{ nullptr };
		/* @brief Swapchain handler */
		vulkan::Swapchain* pSwapchain{ nullptr };
		/* @brief The renderer */
		vulkan::Renderer* pRenderer{ nullptr };

		/* @brief Drawing matrices that are kept uptodate to the current extent */
		struct {
			DirectX::XMFLOAT4X4 orthoLH;
			DirectX::XMFLOAT4X4 orthoRH;
			DirectX::XMFLOAT4X4 projLH;
			DirectX::XMFLOAT4X4 projRH;
			DirectX::XMFLOAT4X4 baseViewLH;
			DirectX::XMFLOAT4X4 baseViewRH;
		} matrices;

		/* @brief Holds settings for the vulkan handling */
		struct {
#ifdef _DEBUG
			bool validation{ true };
#else 
			bool validation{ false };
#endif
			float nearPlane{ 0.1f };
			float farPlane{ 1000.0f };
		} settings;

		/// Allocated resource tracking
		std::vector<uint32_t*> iCurrentMeshes;
		std::vector<EE::Mesh*> currentMeshes;

		std::vector<uint32_t*> iCurrentBuffers;
		std::vector<EE::Buffer*> currentBuffers;

		std::vector<uint32_t*> iCurrentTextures;
		std::vector<EE::Texture*> currentTextures;

		std::vector<uint32_t*> iCurrentShader;
		std::vector<EE::Shader*> currentShader;

		std::vector<uint32_t*> iCurrentObjects;
		std::vector<EE::Object*> currentObjects;

		/// Predefined shader
		struct {
			/***********************VERTEX***********************
			 * #version 450
			 * #extension GL_ARB_separate_shader_objects : enable
			 *
			 * layout(location = 0) in vec3 position;
			 *
			 * layout(binding = 0) uniform UBO {
			 *	 mat4 ortho;
			 *	 mat4 baseView;
			 *	 mat4 world;
			 * } ubo;
			 * 
			 * void main() {
			 * 	 gl_Position = ubo.ortho * ubo.baseView * ubo.world * vec4(position, 1.0);
			 * }
			 *
			 **********************FRAGMENT**********************
			 * #version 450
			 * #extension GL_ARB_separate_shader_objects : enable
			 * 
			 * layout(location = 0) out vec4 outColor;
			 * 
			 * layout(binding = 1) uniform UBO {
			 * 	vec4 bgColor;
			 * } ubo;
			 * 
			 * void main() {
			 * 	outColor = ubo.bgColor;
			 * }
			 **/
			EEShader color2D{ nullptr };
		} shader;


		/**
		 * Default constructor
		 **/
		Graphics();

		/* Destructor */
		~Graphics();

		/**
		 * Creates the necessary vulkan types:
		 *   - VulkanInstance
		 *
		 * @param pWindow		The window the graphics will be created for
		 * @param appCInfo	Create info of the application since some of this is needed in here
		 *
		 * @return Indicates if everything went correct, but most of the time when an error occured
		 *		assert is triggered anyway.
		 **/
		bool Create(Window* pWindow, EEApplicationCreateInfo const& appCInfo);

		/**
		 * Draws all current objects
		 **/
		void Draw();

		/**
		 * Recreates/Reconfigures the renderer to the new current size
		 **/
		void Resize();

		/* @brief Create methods for any type of vulkan resource representation */
		EEMesh CreateMesh(void const* pVertices, size_t amountVertices, std::vector<uint32_t> const& indices);
		EEBuffer CreateBuffer(size_t bufferSize);
		EETexture CreateTexture(char const* fileName, bool enableMipMapping, bool unnormalizedCoordinates);
		EETexture CreateTexture(EETextureCreateInfo const& textureCInfo);
		EEShader CreateShader(EEShaderCreateInfo const& shaderCInfo);
		EEObject CreateObject(EEShader shader, EEMesh mesh, std::vector<EEObjectResourceBinding> const& bindings, EESplitscreen	splitscreen);

		/* @brief Release methods for any type of vulkan resource representation */
		void ReleaseObject(EEObject&);
		void ReleaseMesh(EEMesh&);
		void ReleaseShader(EEShader&);
		void ReleaseTexture(EETexture&);
		void ReleaseBuffer(EEBuffer&);

		/* @brief Update methods for buffer and mesh */
		void UpdateBuffer(EEBuffer buffer, void const* pData);
		void UpdateMesh(EEMesh, void const* pVertices, size_t bufferSize, std::vector<uint32_t> const& indices);


		void vk_instance();
		void vk_device();
		void vk_debug();
		void vk_swapchain();
		void vk_renderer(EEApplicationCreateInfo const&);

	};

}