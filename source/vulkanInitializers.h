/////////////////////////////////////////////////////////////////////
// Filename: vulkanInitializers.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

// VULKAN INCLUDE //
#include <vulkan/vulkan.h>

namespace EE {
	namespace vulkan {
		namespace initializers {

			inline VkSemaphoreCreateInfo semaphoreCreateInfo()
			{
				VkSemaphoreCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				info.pNext = nullptr;
				info.flags = 0;
				return info;
			}

			inline VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCInfo()
			{
				VkPipelineInputAssemblyStateCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				info.pNext = nullptr;
				info.flags = 0;
				info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				info.primitiveRestartEnable = false;
				return info;
			}

			inline VkMemoryAllocateInfo memoryAllocateInfo()
			{
				VkMemoryAllocateInfo memInfo;
				memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memInfo.pNext = nullptr;
				memInfo.allocationSize = 0u;
				memInfo.memoryTypeIndex = 0u;
				return memInfo;
			}

			inline VkMappedMemoryRange mappedMemoryRange(
				VkDeviceMemory memory,
				VkDeviceSize	 size)
			{
				VkMappedMemoryRange mappedMemoryRange;
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.pNext = nullptr;
				mappedMemoryRange.memory = memory;
				mappedMemoryRange.offset = 0;
				mappedMemoryRange.size = size;
				return mappedMemoryRange;
			}

			inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
				VkCommandPool					cmdPool,
				VkCommandBufferLevel	level,
				uint32_t							bufferCount)
			{
				VkCommandBufferAllocateInfo cmdBufferAllocInfo;
				cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cmdBufferAllocInfo.pNext = nullptr;
				cmdBufferAllocInfo.commandPool = cmdPool;
				cmdBufferAllocInfo.level = level;
				cmdBufferAllocInfo.commandBufferCount = bufferCount;
				return cmdBufferAllocInfo;
			}

			inline VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags)
			{
				VkCommandBufferBeginInfo beginInfo;
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.pNext = nullptr;
				beginInfo.flags = flags;
				beginInfo.pInheritanceInfo = nullptr;
				return beginInfo;
			}

			inline VkSubmitInfo submitInfo(
				VkCommandBuffer* commandBuffers,
				uint32_t				 commandBufferCount)
			{
				VkSubmitInfo submitInfo;
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.pNext = nullptr;
				submitInfo.waitSemaphoreCount = 0u;
				submitInfo.pWaitSemaphores = nullptr;
				submitInfo.pWaitDstStageMask = nullptr;
				submitInfo.commandBufferCount = commandBufferCount;
				submitInfo.pCommandBuffers = commandBuffers;
				submitInfo.signalSemaphoreCount = 0u;
				submitInfo.pSignalSemaphores = nullptr;
				return submitInfo;
			}

			inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags)
			{
				VkFenceCreateInfo fenceCInfo;
				fenceCInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceCInfo.pNext = nullptr;
				fenceCInfo.flags = flags;
				return fenceCInfo;
			}

			inline VkDescriptorPoolSize descriptorPoolSize(
				VkDescriptorType type,
				uint32_t				 descriptorCount)
			{
				VkDescriptorPoolSize descriptorPoolSize;
				descriptorPoolSize.type = type;
				descriptorPoolSize.descriptorCount = descriptorCount;
				return descriptorPoolSize;
			}

			inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
				VkDescriptorType	 type,
				VkShaderStageFlags stageFlags,
				uint32_t					 binding,
				uint32_t					 descriptorCount = 1u)
			{
				VkDescriptorSetLayoutBinding setLayoutBinding;
				setLayoutBinding.binding = binding;
				setLayoutBinding.descriptorType = type;
				setLayoutBinding.descriptorCount = descriptorCount;
				setLayoutBinding.stageFlags = stageFlags;
				setLayoutBinding.pImmutableSamplers = nullptr;
				return setLayoutBinding;
			}

			inline VkWriteDescriptorSet writeDescriptorSet(
				VkDescriptorSet					dstSet,
				VkDescriptorType				type,
				uint32_t								binding,
				VkDescriptorBufferInfo* bufferInfo,
				uint32_t								descriptorCount = 1u)
			{
				VkWriteDescriptorSet writeDescriptorSet;
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.pNext = nullptr;
				writeDescriptorSet.dstSet = dstSet;
				writeDescriptorSet.dstBinding = binding;
				writeDescriptorSet.dstArrayElement = 0u;
				writeDescriptorSet.descriptorCount = descriptorCount;
				writeDescriptorSet.descriptorType = type;
				writeDescriptorSet.pImageInfo = nullptr;
				writeDescriptorSet.pBufferInfo = bufferInfo;
				writeDescriptorSet.pTexelBufferView = nullptr;
				return writeDescriptorSet;
			}

			inline VkWriteDescriptorSet writeDescriptorSet(
				VkDescriptorSet				 dstSet,
				VkDescriptorType			 type,
				uint32_t							 binding,
				VkDescriptorImageInfo* imageInfo,
				uint32_t							 descriptorCount = 1u)
			{
				VkWriteDescriptorSet writeDescriptorSet;
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.pNext = nullptr;
				writeDescriptorSet.dstSet = dstSet;
				writeDescriptorSet.dstBinding = binding;
				writeDescriptorSet.dstArrayElement = 0u;
				writeDescriptorSet.descriptorCount = descriptorCount;
				writeDescriptorSet.descriptorType = type;
				writeDescriptorSet.pImageInfo = imageInfo;
				writeDescriptorSet.pBufferInfo = nullptr;
				writeDescriptorSet.pTexelBufferView = nullptr;
				return writeDescriptorSet;
			}

			inline VkBufferCreateInfo bufferCreateInfo(
				VkBufferUsageFlags usage,
				VkDeviceSize			 size)
			{
				VkBufferCreateInfo bufferCInfo;
				bufferCInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCInfo.pNext = nullptr;
				bufferCInfo.flags = 0;
				bufferCInfo.size = size;
				bufferCInfo.usage = usage;
				bufferCInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				bufferCInfo.queueFamilyIndexCount = 0u;
				bufferCInfo.pQueueFamilyIndices = nullptr;
				return bufferCInfo;
			}

			inline VkBufferImageCopy bufferImageCopy(
				VkImageAspectFlagBits aspectMask,
				uint32_t							width,
				uint32_t							height)
			{
				VkBufferImageCopy copyRegion;
				copyRegion.bufferOffset = 0;
				copyRegion.bufferRowLength = 0u;
				copyRegion.bufferImageHeight = 0u;
				copyRegion.imageSubresource.aspectMask = aspectMask;
				copyRegion.imageSubresource.mipLevel = 0u;
				copyRegion.imageSubresource.baseArrayLayer = 0u;
				copyRegion.imageSubresource.layerCount = 1u;
				copyRegion.imageOffset = { 0u, 0u, 0u };
				copyRegion.imageExtent = { width, height, 1u };
				return copyRegion;
			}
		}
	}
}