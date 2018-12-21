/////////////////////////////////////////////////////////////////////
// Filename: graphicsClass.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanInstance.h"


namespace EE {

	namespace vulkan {
		///////////////////////////
		// FOREWARD DECLARATIONS //
		///////////////////////////
		struct VulkanDevice;

		// The use of this struct wrapping around a simple command buffer is
		// that the Vulkan Device will control which queue this command buffer is submitted to.
		// The execution can be modified by passing in a pointer to a submit info.
		// A nullptr will use a default submit info and wait for completed execution.
		struct ExecBuffer {
			VkCommandBuffer cmdBuffer;
			VkQueue queue;
			EE::vulkan::VulkanDevice* device;

			void Execute(VkSubmitInfo* submitInfo = nullptr, bool wait = true, bool free = true);
		};
	}

	struct Graphics
	{
		/* @brief The window this graphics are created for */
		Window* pWindow{ nullptr };
		/* @brief The VkAllocationCallbacks currently in use (no one) */
		VkAllocationCallbacks const* pAllocator{ nullptr };
		/* @brief A struct that manages handling the vulkan instance */
		vulkan::Instance* pInstance{ nullptr };

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


	};

}