/////////////////////////////////////////////////////////////////////
// Filename: graphicsClass.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "graphicsClass.h"

#include <cassert>

using namespace EE;

void vulkan::ExecBuffer::Execute(VkSubmitInfo* _submitInfo, bool wait, bool free)
{
	/* @TODO Structures/Features used by this method not yet implemented
	assert(cmdBuffer);

	VK_CHECK(vkEndCommandBuffer(cmdBuffer));

	// Submit info (passed in will be used if not nullptr)
	VkSubmitInfo submitInfo;
	if (_submitInfo) submitInfo = *_submitInfo;
	else submitInfo = vkee::initializers::submitInfo(&cmdBuffer, 1u);

	// Create fence if we wanna wait
	VkFence fence{ VK_NULL_HANDLE };
	if (wait)
	{
		VkFenceCreateInfo fenceCInfo = vkee::initializers::fenceCreateInfo(VK_FLAGS_NONE);
		VK_CHECK(vkCreateFence(device->logicalDevice, &fenceCInfo, device->pAllocator, &fence));
	}

	// Submit to the queue
	vkQueueSubmit(queue, 1u, &submitInfo, fence);

	// Wait for the fence to signal that the cmd buffer has finished execution and destroy afterwards (if desired)
	if (fence != VK_NULL_HANDLE)
	{
		VK_CHECK(vkWaitForFences(device->logicalDevice, 1u, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		vkDestroyFence(device->logicalDevice, fence, device->pAllocator);
	}

	if (free)
	{
		vkFreeCommandBuffers(device->logicalDevice, device->cmdGraphicsPool, 1u, &cmdBuffer);
	}
	*/
}