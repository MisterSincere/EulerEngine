/////////////////////////////////////////////////////////////////////
// Filename: Graphics.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vkcore/vulkanRenderer.h"


namespace EE {

	struct Shader;

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

		/* @brief Holds settings for the vulkan handling */
		struct {
#ifdef _DEBUG
			bool validation{ true };
#else 
			bool validation{ false };
#endif
		} settings;

		std::vector<uint32_t*> iCurrentShader;
		std::vector<EE::Shader*> currentShader;


		/**
		 * Default constructor
		 **/
		Graphics();

		/**
		 * Default destructor
		 **/
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
		 * Creates a shader and returns a handle to it
		 * @param shaderCInfo
		 **/
		EEShader CreateShader(EEShaderCreateInfo const& shaderCInfo);

		void vk_instance();
		void vk_device();
		void vk_debug();
		void vk_swapchain();
		void vk_renderer(EEApplicationCreateInfo const&);

	};

}