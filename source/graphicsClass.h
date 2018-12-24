/////////////////////////////////////////////////////////////////////
// Filename: graphicsClass.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDevice.h"


namespace EE {


	struct Graphics
	{
		/* @brief The window this graphics are created for */
		Window* pWindow{ nullptr };
		/* @brief The VkAllocationCallbacks currently in use (no one) */
		VkAllocationCallbacks const* pAllocator{ nullptr };
		/* @brief A struct that manages handling the vulkan instance */
		vulkan::Instance* pInstance{ nullptr };
		/* @brief Struct that manages physical and logical device and command buffer allocation (queues) */
		vulkan::Device* pDevice{ nullptr };

		/* @brief Holds settings for the vulkan handling */
		struct {
#ifdef _DEBUG
			bool validation{ true };
#else 
			bool validation{ false };
#endif
		} settings;


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
		 *
		 * @return Indicates if everything went correct, but most of the time when an error occured
		 *		assert is triggered anyway.
		 **/
		bool Create(Window* pWindow);



		void vk_instance();
		void vk_device();

	};

}