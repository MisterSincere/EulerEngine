/////////////////////////////////////////////////////////////////////
// Filename: vulkanDebug.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanInstance.h" //< vulkanTools.h vulkanInitializers.h vulkan.h

namespace EE
{
	namespace vulkan
	{
		struct Debug
		{
			/* @brief The instance of the application */
			Instance* pInstance;

			/* @brief Extension functions */
			PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT{ nullptr };
			PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT{ nullptr };
			PFN_vkDebugReportMessageEXT fpDebugReportMessageEXT{ nullptr };

			/* @brief Handle defining the debuf callback */
			VkDebugReportCallbackEXT callback{ VK_NULL_HANDLE };

			/**
			 * Default constructor
			 *
			 * @param pInstance		Instance this debug callbacks will be used for
			 **/
			Debug(Instance* pInstance);

			/**
			 * Default destructor
			 **/
			~Debug();

			/**
			 * Creates the debug callback association for the instance
			 **/
			void Create();


			/*@brief Delete dangerous move/copy constructors etc. */
			Debug(Debug const&) = delete;
		};
	}
}