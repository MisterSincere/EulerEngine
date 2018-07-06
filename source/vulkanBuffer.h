/////////////////////////////////////////////////////////////////////
// Filename: vulkanInitializers.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDebug.h" //< vulkanDefs.h, vulkan.h

#include <assert.h>
#include <cstring>

namespace vk
{
  /**
   * @brief Encapsulates access to a vulkan buffer backed up by device memory
   * @note Needs to be filled by an external source like the VulkanDevice
   **/
  struct Buffer
  {
    VkDevice device;
    const VkAllocationCallbacks* pAllocator;
    VkBuffer buffer{ VK_NULL_HANDLE };
    VkDeviceMemory memory{ VK_NULL_HANDLE };
    VkDescriptorBufferInfo descriptor;
    VkDeviceSize size{ 0 };
    VkDeviceSize alignment{ 0 };
    void* mapped{ nullptr };

    /* @brief Usage flags of this buffer that are set when creating the buffer (could become handy later) */
    VkBufferUsageFlags usageFlags;
    /* @brief Memory property flags of this buffer that are set when creating the buffer (could become handy later) */
    VkMemoryPropertyFlags memoryPropertyFlags;

    /**
     * Map a memory range of this buffer.
     *
     * @param size      The size of the memory to be mapped. VK_WHOLE_SIZE to map the whole buffer (optional)
     * @param offset    Byte offset from the beginning (optional)
     *
     * @return VkResult of the buffer mapping call
     **/
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    {
      return vkMapMemory(device, memory, offset, size, 0, &mapped);
    }

    /**
     * Unmap a mapped memory range.
     *
     * @note No result that needs to return, since vkUnmapMemory can't fail
     **/
    void unmap()
    {
      if (mapped)
      {
        vkUnmapMemory(device, memory);
        mapped = nullptr;
      }
    }

    /**
     * Attach the allocated memory to the created buffer.
     *
     * @param offset  Byte offset from the beginning for the memory region to bind (optional)
     *
     * @return VkResult of the vkBindBufferMemory call
     **/
    VkResult bind(VkDeviceSize offset = 0)
    {
      return vkBindBufferMemory(device, buffer, memory, offset);
    }

    /**
     * Set up the default desriptor for this buffer.
     *
     * @param size    Size of the memory range of the descriptor (optional)
     * @param offset  Byte offset from the beginning (optional)
     **/
    void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    {
      descriptor.offset = offset;
      descriptor.buffer = buffer;
      descriptor.range = size;
    }

    /**
     * Copies the passed in data to the mapped buffer
     *
     * @param data  Pointer to the data to copy
     * @param size  Size of the data to copy
     **/
    void copyTo(void* data, VkDeviceSize size)
    {
      assert(mapped);
      memcpy(mapped, data, size);
    }

    /**
     * Flush a memory range of the memory to make it visible for the device.
     *
     * @note Only required for non-coherent memory
     *
     * @param size      Size of the memory range to flush (optional)
     * @param offset    Byte offset from the beginning (optional)
     *
     * @return VkResult from the flush call
     **/
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    {
      VkMappedMemoryRange mappedRange;
      mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      mappedRange.pNext = nullptr;
      mappedRange.memory = memory;
      mappedRange.offset = offset;
      mappedRange.size = size;
      return vkFlushMappedMemoryRanges(device, 1u, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host.
     *
     * @note Only required for non-coherent memory
     *
     * @param size    Size of the memory range to invalidate (optional)
     * @param offset  Byte offset from the beginning (optional)
     *
     * @return VkResult of the invalidate call
     **/
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
    {
      VkMappedMemoryRange mappedRange;
      mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      mappedRange.pNext = nullptr;
      mappedRange.memory = memory;
      mappedRange.offset = offset;
      mappedRange.size = size;
      return vkInvalidateMappedMemoryRanges(device, 1u, &mappedRange);
    }

    /**
     * Release all vulkan resources held by this buffer.
     **/
    void release()
    {
      if (buffer)
      {
        vkDestroyBuffer(device, buffer, pAllocator);
        buffer = VK_NULL_HANDLE;
      }
      if (memory)
      {
        vkFreeMemory(device, memory, pAllocator);
        memory = VK_NULL_HANDLE;
      }
    }
  };
}