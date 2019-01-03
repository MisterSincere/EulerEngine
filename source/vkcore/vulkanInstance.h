/////////////////////////////////////////////////////////////////////
// Filename: vulkanInstance.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanTools.h"
#include "Window.h"

namespace EE
{
	namespace vulkan
	{
		struct Instance
		{
			/* @brief Vulkan's per-application instance */
			VkInstance instance{ VK_NULL_HANDLE };
			/* @brief Pointer to the different allocation callbacks */
			VkAllocationCallbacks const* pAllocator{ nullptr };

			/* @brief List of supported layers */
			std::vector<VkLayerProperties> supportedLayers;
			/* @brief Lists with names of the enabled layers */
			std::vector<char const*> enabledLayers;

			/* @brief List of supported extensions */
			std::vector<VkExtensionProperties> supportedExtensions;
			/* @brief List with names of the enabled extensions */
			std::vector<char const*> enabledExtensions;

			/* @brief Internal identification name that is needed when creating a vulkan instance */
			char const* name{ "MS:EulerEngine" };

			/* @brief Max supported api version */
			uint32_t maxApiVersion{ VK_API_VERSION_1_0 };
			/* @brief Use api version */
			uint32_t apiVersion{ VK_API_VERSION_1_0 };

			/* @brief Typecast to a VkInstance */
			operator VkInstance() const { return instance; }

			/**
			 * Default constructor that stores the settings and checks if they are supported.
			 *
			 * @param window								The window instance the instance will be used for (compatibility check)
			 * @param additionalLayers			Additional layers that are desired (will be checked)
			 * @param additionalExtensions	Additional extensions that are desired (will be checked)
			 *
			 * @note Layers and extensions that are required for this instance according to work for the window
			 *			 are already checked on window creation time.
			 **/
			Instance(
				EE::Window const&				 window,
				std::vector<char const*> additionalLayers,
				std::vector<char const*> additionalExtensions);

			/**
			 * Default desctructor
			 **/
			~Instance();

			/**
			 * Creates the instance according to the layers and extensions set with the constructor
			 *
			 * @param pAllocator		The allocation callbacks (can be nullptr for lazy alloc)
			 *
			 * @return VkResult of the vkCreateInstance call
			 **/
			VkResult Create(VkAllocationCallbacks const* pAllocator);


			/* @brief Delete copy/move constructor/assignements */
			Instance(Instance const&) = delete;
			Instance(Instance&&) = delete;
			Instance& operator=(Instance const&) = delete;
			Instance& operator=(Instance&&) = delete;
		};

	}
}