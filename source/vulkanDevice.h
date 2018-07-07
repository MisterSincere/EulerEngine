/////////////////////////////////////////////////////////////////////
// Filename: vulkanDevice.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanBuffer.h" //< vulkanDebug.h, vulkanDefs.h, vulkan.h
#include "vulkanInstance.h"

#include "window.h"
#include "eedefs.h"

#include <vector>
#include <set>

namespace vk
{
  struct VulkanDevice
  {
    vk::VulkanInstance* instance;
    eewindow::Window* window;

    /* @brief The representation of the picked physical device (gpu) */
    VkPhysicalDevice physicalDevice;
    /* @brief The allocation callbacks */
    const VkAllocationCallbacks* pAllocator;
    /* @brief Logical representation of the gpu (application's view of the gpu) */
    VkDevice logicalDevice;
    /* @brief Properties of the gpu including limits that the application may check against */
    VkPhysicalDeviceProperties properties;
    /* @brief Features of the physical device that the application can check against to see if a feature is supported */
    VkPhysicalDeviceFeatures supportedFeatures;
    /* @brief Features that have been enabled from the application */
    VkPhysicalDeviceFeatures enabledFeatures;
    /* @brief Memory types and heaps of the physical device */
    VkPhysicalDeviceMemoryProperties memoryProperties;
    /* @brief Queue family properties of the physical device */
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;

    /* @brief List of the layers that are supported */
    std::vector<VkLayerProperties> supportedLayers;
    /* @brief List of enabled layers */
    std::vector<const char*> enabledLayers;

    /* @brief List of extensions that are supported */
    std::vector<VkExtensionProperties> supportedExtensions;
    /* @brief List of the extension the device uses */
    std::vector<const char*> enabledExtensions;

    /* @brief The default command pool for graphic queues */
    VkCommandPool cmdGraphicsPool{ VK_NULL_HANDLE };

    /* @brief Contains the queue family indices */
    struct
    {
      uint32_t graphics;
      uint32_t graphicsCount;
      uint32_t compute;
      uint32_t computeCount;
      uint32_t transfer;
      uint32_t transferCount;
    } queueFamilyIndices;

    /* @brief Typecast to VkDevice */
    operator VkDevice() { return logicalDevice; }

    /**
     * Default constructor:
     *  - picks physical devices compatible to the passed in window
     *  - stores data of the picked physical device (features, layer, extensions etc.)
     *
     * @param instance    The instance struct describing
     * @param window      The window the device needs to be compatible with
     * @param pAllocator  The allocation callbacks (can be nullptr to use lazy alloc)
     **/
    VulkanDevice(vk::VulkanInstance* instance, eewindow::Window* window, const VkAllocationCallbacks* pAllocator);

    /**
     * Default destructor
     **/
    ~VulkanDevice();
    
    /**
     * Create the logical device based on the assigned phyisical device, also gets default queue family indices
     *
     * @param additionalLayers       Layer to activate additional to the required ones
     * @param additionalExtensions   Extensions to activate additional to the required ones by the window
     * @param requestedQueueTypes    Bitmask speciyfying the queue types that will be reqeuested from the logical device (optional)
     *
     * @return VkResult of the creation call
     **/
    VkResult Create(
      const VkPhysicalDeviceFeatures& desiredFeatures,
      std::vector<const char*>& additionalLayers,
      std::vector<const char*>& additionalExtensions,
      VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

    /**
     * Returns a queue of the family passed in.
     *
     * @param requestedQueueType  The type of the queue to return
     *
     * @note Just pass in one type, hence no bitwise or
     **/
    VkQueue GetQueue(VkQueueFlags requestedQueueType);

    /**
     * Create a buffer on the device
     *
     * @param usageFlags            Usage flag bitmask for the buffer (i.e. index, vertex, uniform buffer)
     * @param memoryPropertyFlags   Memory properties for this buffer (i.e. device local, host visible, coherent)
     * @param size                  Size of the buffer in bytes
     * @param buffer                Pointer to the buffer handle acquired by this function
     * @param memory                Pointer to the memory handle acquired by this function
     * @param data                  Pointer to data that should be copied to the buffer after creation (optional)
     *
     * @return VK_SUCCESS if the buffer handle and memory have been created
     **/
    VkResult CreateBuffer(
      VkBufferUsageFlags    usageFlags,
      VkMemoryPropertyFlags memPropertyFlags,
      VkDeviceSize          size,
      VkBuffer*             buffer,
      VkDeviceMemory*       memory,
      void*                 data = nullptr);

    /**
     * Create a buffer on the device.
     *
     * @param usageFlags            The usage flags for the buffer (i.e. index, vertex, uniform buffer)
     * @param memoryPropertyFlags   Memory properties for this buffer (i.e. device local, host visible, coherent)
     * @param buffer                Pointer to a vk::Buffer object
     * @param size                  Size of the buffer in bytes
     * @param data                  Pointer to the data that should be copied to the buffer after creation (optional)
     *
     * @return VK_SUCCESS if buffer handle and memory have benn created (and optionally) passed data has been copied
     **/
    VkResult CreateBuffer(
      VkBufferUsageFlags    usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags,
      vk::Buffer*           buffer,
      VkDeviceSize          size,
      void*                 data = nullptr);

    /**
     * Copy buffer data from src to dst using vkCmdCopyBuffer
     *
     * @param src         Pointer to the source buffer to copy from
     * @param dst         Pointer to the destination buffer to copy to
     * @param queue       The queue the copy will be executed on
     * @param copyRegion  Pointer to the copy region, if nullptr the whole buffer will be copied (optional)
     *
     * @note Source and destination buffer must have appropriate transfer usage flags set (TRANSFER_SRC / TRANSFER_DST)
     **/
    void CopyBuffer(
      vk::Buffer*   src,
      vk::Buffer*   dst,
      VkQueue       queue,
      VkBufferCopy* copyRegion = nullptr);

    /**
     * Allocate a command buffer from the command pool
     *
     * @param level     Level of the command buffer (primary or secondary)
     * @param begin     If true the command buffer will already begin recording (defaults to false)
     *
     * @return A handle to the allocated command buffer
     **/
    VkCommandBuffer CreateCommandBuffer(
      VkCommandBufferLevel level,
      bool                 begin = false);

    /**
     * Finish command buffer recording and submit it to a queue.
     *
     * @param commandBuffer   The command buffer that should be flushed
     * @param queue           The queue the command buffer will be submitted on
     * @param free            Free the command buffer once it has been submitted (Defaults to true)
     *
     * @note The queeu that the command buffer is submitted on must be from the same family index as the pool it was allocated from
     * @note Uses a fence to ensure the command buffer has finished executing
     **/
    void FlushCommandBuffer(
      VkCommandBuffer cmdBuffer,
      VkQueue         queue,
      bool            free = true);

    /**
     * Get the index of the memory type that has all the requested property flags.
     *
     * @param typeBits      Bitmask with bits set for eacm memory type supported by the resource to request for (VkMemoryRequirements)
     * @param properties    Bitmask of properties for the memory type that is requested
     * @param memTypeFound  Pointer to a boolean that is set to true if a matching memory type has been found (optional)
     * 
     * @return Index of the requested memory type
     *
     * @note Throws an exception if memTypeFound is a nullptr and no memory type was found
     **/
    uint32_t GetMemoryType(
      uint32_t              typeBits,
      VkMemoryPropertyFlags properties,
      VkBool32*             memTypeFound = nullptr);

    /**
     * Gets the index of the queue family that supported the passed in flags
     *
     * @param queueFlags    The flags determining the queue to find
     *
     * @return Index of the desired queue family
     *
     * @throw Throws an exception if no queue family index could be found according to the param
     **/
    uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);

    /**
     * Create a command pool to allocate command buffers from
     *
     * @param queueFamilyIndex    Family index of the queeu the command buffer will execute on
     * @param createFlags         Command pool creation flags (Default to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
     *
     * @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
     *
     * @return A handle to the created command pool
     **/
    VkCommandPool createCommandPool(
      uint32_t                queueFamilyIndex,
      VkCommandPoolResetFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    /**
     * Check if an layer is supported by the physical device.
     *
     * @param layer       The name of the layer to check
     *
     * @return Is true if the layer is supported
     **/
    bool layerSupported(const char* layer);

    /**
     * Check if an extension is supported by the physical device.
     *
     * @param extension   The name of the extension to check
     *
     * @return Is true if the extension is supported
     **/
    bool extensionSupported(const char* extension);

    /**
     * Checks for every available gpu if it is adequate to the window
     *
     * @param instance    The instance we want to check against
     *
     * @return The handle to the suitable physical device
     **/
    VkPhysicalDevice pickPhysicalDevice(VkInstance instance);
  };
}