/////////////////////////////////////////////////////////////////////
// Filename: vulkanInitializers.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <vulkan/vulkan.h>

namespace vk
{
  namespace initializers
  {
    inline VkBufferCreateInfo bufferCreateInfo(
      VkBufferUsageFlags usage,
      VkDeviceSize size)
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

    inline VkMemoryAllocateInfo memoryAllocateInfo()
    {
      VkMemoryAllocateInfo allocInfo;
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.pNext = nullptr;
      allocInfo.allocationSize = 0u;
      allocInfo.memoryTypeIndex = 0u;
      return allocInfo;
    }

    inline VkMappedMemoryRange mappedMemoryRange(
      VkDeviceMemory memory,
      VkDeviceSize size)
    {
      VkMappedMemoryRange mmr;
      mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      mmr.pNext = nullptr;
      mmr.memory = memory;
      mmr.offset = 0;
      mmr.size = size;
      return mmr;
    }

    inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
      VkCommandPool cmdPool,
      VkCommandBufferLevel level,
      uint32_t bufferCount)
    {
      VkCommandBufferAllocateInfo info;
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      info.pNext = nullptr;
      info.commandPool = cmdPool;
      info.level = level;
      info.commandBufferCount = bufferCount;
      return info;
    }
    
    inline VkCommandBufferBeginInfo commandBufferBeginInfo(
      VkCommandBufferUsageFlags flags)
    {
      VkCommandBufferBeginInfo info;
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.pNext = nullptr;
      info.flags = flags;
      info.pInheritanceInfo = nullptr;
      return info;
    }

    inline VkSubmitInfo submitInfo(
      VkCommandBuffer* commandBuffers,
      uint32_t commandBufferCount)
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

    inline VkFenceCreateInfo bufferCreateInfo(
      VkFenceCreateFlags flags)
    {
      VkFenceCreateInfo info;
      info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      info.pNext = nullptr;
      info.flags = flags;
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

    inline VkViewport viewport(
      uint32_t width,
      uint32_t height)
    {
      VkViewport vp;
      vp.x = 0.0f;
      vp.y = 0.0f;
      vp.width = static_cast<float>(width);
      vp.height = static_cast<float>(height);
      vp.minDepth = 0.0f;
      vp.maxDepth = 1.0f;
      return vp;
    }

    inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
      VkDescriptorType type,
      VkShaderStageFlags stageFlags,
      uint32_t binding,
      uint32_t descriptorCount = 1u)
    {
      VkDescriptorSetLayoutBinding setLayoutBinding;
      setLayoutBinding.binding = binding;
      setLayoutBinding.descriptorType = type;
      setLayoutBinding.descriptorCount = descriptorCount;
      setLayoutBinding.stageFlags = stageFlags;
      setLayoutBinding.pImmutableSamplers = nullptr;
      return setLayoutBinding;
    }

    inline VkDescriptorPoolSize descriptorPoolSize(
      VkDescriptorType type,
      uint32_t descriptorCount)
    {
      VkDescriptorPoolSize info;
      info.type = type;
      info.descriptorCount = descriptorCount;
      return info;
    }
  }
}