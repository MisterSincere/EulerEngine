/////////////////////////////////////////////////////////////////////
// Filename: graphicsClass.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanTools.h"


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

	class Graphics {

	};

}