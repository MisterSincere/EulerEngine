/////////////////////////////////////////////////////////////////////
// Filename: vulkanDevice.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanBuffer.h" //< vulkanDebug.h, vulkanDefs.h, vulkan.h

#include "eedefs.h"

#include <vector>
#include <set>

namespace vk
{
  namespace physicalDevice
  {
    
  }

  struct VulkanDevice
  {
    /* @brief The representation of the picked physical device (gpu) */
    VkPhysicalDevice physicalDevice;
    /* @brief The allocation callbacks */
    const VkAllocationCallbacks* pAllocator;
    /* @brief Logical representation of the gpu (application's view of the gpu) */
    VkDevice logicalDevice;
    /* @brief Properties of the gpu including limits that the application may check against */
    VkPhysicalDeviceProperties properties;
    /* @brief Features of the physical device tha the application can check against to see if a feature is supported */
    VkPhysicalDeviceFeatures features;
    /* @brief Features that have been enabled from the application */
    VkPhysicalDeviceFeatures enabledFeatures;
    /* @brief Memory types and heaps of the physical device */
    VkPhysicalDeviceMemoryProperties memoryProperties;
    /* @brief Queue family properties of the physical device */
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    /* @brief List of extensions that are supported */
    std::vector<const char*> supportedExtensions;
    /* @brief List of the extension the device uses */
    std::vector<const char*> enabledExtensions;

    /* @brief The default command pool for graphic queues */
    VkCommandPool cmdGraphicsPool{ VK_NULL_HANDLE };

    /* @brief Set to true if the debug marker extension was detected */
    bool enableDebugMarkes{ false };

    /* @brief Contains the queue family indices */
    struct
    {
      uint32_t graphics;
      uint32_t compute;
      uint32_t transfer;
    } queueFamilyIndices;

    /* @brief Typecast to VkDevice */
    operator VkDevice() { return logicalDevice; }

    /**
     * Default constructor
     *
     * @param instance    The instance the device will be created on
     * @param surface     The surface the device must be compatible with
     * @param pAllocator  The allocation callbacks (can be nullptr to use lazy alloc)
     * @param extensions  The extensions to activate additionally to VK_KHR_SWAPCHAIN_EXTENSIONS_NAME that is automatically added
     **/
    VulkanDevice(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator, std::vector<const char*>& desiredExtensions = std::vector<const char*>(0))
      : pAllocator(pAllocator)
    {
      // Pick physical device
      desiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      physicalDevice = pickPhysicalDevice(instance, surface, desiredExtensions);

      // Store the desired extensions as enabled
      for (size_t i = 0; i < desiredExtensions.size(); i++)
      {
        enabledExtensions.push_back(desiredExtensions[i]);
      }

      // Store properties features, limits and properties of the physical device for later use
      // Device properties also contain limits and sparse properties
      vkGetPhysicalDeviceProperties(physicalDevice, &properties);
      // Store supported device features
      vkGetPhysicalDeviceFeatures(physicalDevice, &features);
      // Memory properties are used for creating all kinds of buffers
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
      // Get the queue family properties, needed information when creating the device or command pool
      uint32_t queueFamilyCount;
      vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
      assert(queueFamilyCount > 0);
      queueFamilyProperties.resize(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

      // Get list of the supported extensions
      uint32_t extCount = 0u;
      vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
      if (extCount > 0u)
      {
        std::vector<VkExtensionProperties> extensions(extCount);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()));
        for (auto ext : extensions)
        {
          supportedExtensions.push_back(ext.extensionName);
        }
      }
    }

    /**
     * Checks if the physical device is compatible with the surface and if it supports the extensions passed in
     *
     * @param physicalDevice    The device that is to check
     * @param surface           The surface that the device must be compatible with
     * @param extensions        The extensions the device must support
     *
     * @return True if the passed in device fullfills every criterium
     **/
    bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<const char*>& extensions)
    {
      // Extension support
      bool extensionsSupported{ false };
      {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto& extension : availableExtensions)
        {
          requiredExtensions.erase(extension.extensionName);
        }

        extensionsSupported = requiredExtensions.empty();
      }

      // Swapchain support
      bool swapChainAdequate{ false };
      {
        if (extensionsSupported)
        {
          uint32_t formatCount;
          vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

          uint32_t presentModesCount;
          vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);

          swapChainAdequate = formatCount && presentModesCount;
        }
      }

      return extensionsSupported && swapChainAdequate;
    }

    /**
     * Checks for every available device if it is suitable.
     *
     * @param instance    The instance we want to check against
     * @param surface     The surface we wanna check against
     * @param extensions  The extensions we wanna check against
     *
     * @return The handle to the suitable physical device
     **/
    VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*>& extensions)
    {
      assert(instance);

      uint32_t deviceCount;
      vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
      assert(deviceCount > 0u);
      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

      VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
      for (const auto& device : devices)
      {
        if (isDeviceSuitable(device, surface, extensions))
        {
          physicalDevice = device;
          break;
        }
      }

      if (physicalDevice == VK_NULL_HANDLE)
      {
        EEPRINT("Vulkan Error: Failed to find a suitable GPU!");
        throw std::runtime_error("Vulkan Error: Failed to find a suitable GPU!");
      }

      return physicalDevice;
    }

    /**
     * Default destructor
     **/
    ~VulkanDevice()
    {
      if (cmdGraphicsPool)
      {
        vkDestroyCommandPool(logicalDevice, cmdGraphicsPool, pAllocator);
      }
      if (logicalDevice)
      {
        vkDestroyDevice(logicalDevice, pAllocator);
      }
    }


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
    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr)
    {
      for (uint32_t i = 0u; i < memoryProperties.memoryTypeCount; i++)
      {
        if ((typeBits & 1) == 1)
        {
          if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
          {
            if (memTypeFound) *memTypeFound = VK_TRUE;
            return i;
          }
        }
        typeBits >>= 1;
      }

      if (memTypeFound)
      {
        *memTypeFound = VK_FALSE;
        return 0u;
      }
      else
      {
        EEPRINT("Could not find a matching memory type!\n");
        throw std::runtime_error("Could not find a matching memory type!");
      }
    }

    /**
     * Gets the index of the queue family that supported the passed in flags
     *
     * @param queueFlags    The flags determining the queue to find
     *
     * @return Index of the desired queue family
     *
     * @throw Throws an exception if no queue family index could be found according to the param
     **/
    uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags)
    {
      // Dedicated queue for compute
      // Try to find a queue that supports computation but not graphics
      if (queueFlags & VK_QUEUE_COMPUTE_BIT)
      {
        for (uint32_t i = 0u; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
          if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
          {
            return i;
          }
        }
      }

      // Dedicated queue for transfer
      // Try to find a queue that supports transfer but not graphics and compute
      if (queueFlags & VK_QUEUE_TRANSFER_BIT)
      {
        for (uint32_t i = 0u; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
          if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
          {
            return i;
          }
        }
      }

      // For other queue types or if no dedicated compute/transfer queue is present, return the first one to support the requested flags
      for (uint32_t i = 0u; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
      {
        if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
        {
          return i;
        }
      }

      EEPRINT("Could not find a matching queue family index!\n");
      throw std::runtime_error("Could not find a matching queue family index!");
    }

    /**
     * Create the logical device based on the assigned phyisical device, also gets default queue family indices
     *
     * @param enabledFeatures     Can be used to enable ceratin features upon device creation
     * @param useSwapChain        Set to false for headless rendering to omit the swapchain device extension (optional)
     * @param requestedQueueTypes Bitmask speciyfying the queue types that will be reqeuested from the logical device (optional)
     *
     * @return VkResult of the creation call
     **/
    VkResult createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
      bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)
    {
      // Desired queues need to be requested upon logical device creation
      // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
      // requests different ueue types

      std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

      // Get the queue family indices for the requested family types
      // Note that the indices may overlap depending on the implementation

      const float defaultQueuePriority{ 0.0f };

      // Graphics queue
      if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
      {
        queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueCInfo;
        queueCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCInfo.pNext = nullptr;
        queueCInfo.flags = 0;
        queueCInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueCInfo.queueCount = 1u;
        queueCInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueCInfo);
      }
      else
      {
        queueFamilyIndices.graphics = 0u;
      }

      // Dedicated compute queue
      if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
      {
        queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
        {
          // If compute and graphics family are different we set up another queue for it
          VkDeviceQueueCreateInfo queueCInfo;
          queueCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
          queueCInfo.pNext = nullptr;
          queueCInfo.flags = 0;
          queueCInfo.queueFamilyIndex = queueFamilyIndices.compute;
          queueCInfo.queueCount = 1u;
          queueCInfo.pQueuePriorities = &defaultQueuePriority;
          queueCreateInfos.push_back(queueCInfo);
        }
      }
      else
      {
        queueFamilyIndices.compute = queueFamilyIndices.graphics;
      }

      // Dedicated transfer queue
      if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
      {
        queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
        {
          // If compute, transfer and graphics family are different we set up another queue for it
          VkDeviceQueueCreateInfo queueCInfo;
          queueCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
          queueCInfo.pNext = nullptr;
          queueCInfo.flags = 0;
          queueCInfo.queueFamilyIndex = queueFamilyIndices.transfer;
          queueCInfo.queueCount = 1u;
          queueCInfo.pQueuePriorities = &defaultQueuePriority;
          queueCreateInfos.push_back(queueCInfo);
        }
      }
      else
      {
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
      }

      // Create the logical device representation
      VkDeviceCreateInfo deviceCInfo;
      deviceCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      deviceCInfo.pNext = nullptr;
      deviceCInfo.flags = 0;
      deviceCInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
      deviceCInfo.pQueueCreateInfos = queueCreateInfos.data();

      // Enable debug layer when in debug mode
      std::vector<const char*> deviceLayer;
#ifdef _DEBUG
      deviceLayer.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

      // Enable the debugmarker extension if in debug mode and it is present
#ifdef _DEBUG
      if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
      {
        enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        enableDebugMarkes = true;
      }
#endif

      deviceCInfo.enabledLayerCount = static_cast<uint32_t>(deviceLayer.size());
      deviceCInfo.ppEnabledLayerNames = deviceLayer.data();
      deviceCInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
      deviceCInfo.ppEnabledExtensionNames = enabledExtensions.data();
      deviceCInfo.pEnabledFeatures = &enabledFeatures;

      VkResult result = vkCreateDevice(physicalDevice, &deviceCInfo, pAllocator, &logicalDevice);

      if (result == VK_SUCCESS)
      {
        // Create the default drawing command pool
        cmdGraphicsPool = createCommandPool(queueFamilyIndices.graphics);
      }

      return result;
    }

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
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memPropertyFlags, VkDeviceSize size,
      VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr)
    {
      // Create the buffer handle
      VkBufferCreateInfo bufferCInfo = vk::initializers::bufferCreateInfo(usageFlags, size);
      VK_CHECK(vkCreateBuffer(logicalDevice, &bufferCInfo, pAllocator, buffer));

      // Create the memory backing up the buffer handle
      VkMemoryRequirements memReqs;
      VkMemoryAllocateInfo memAlloc = vk::initializers::memoryAllocateInfo();
      vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
      memAlloc.allocationSize = memReqs.size;
      // Find a memory type index that fits the properties of this buffer
      memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memPropertyFlags);
      VK_CHECK(vkAllocateMemory(logicalDevice, &memAlloc, pAllocator, memory));

      // If a pointer to buffer data has been passed in, map the buffer and copy over the data
      if (data)
      {
        void* mapped;
        VK_CHECK(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
          VkMappedMemoryRange mappedRange = vk::initializers::mappedMemoryRange(*memory, size);
          vkFlushMappedMemoryRanges(logicalDevice, 1u, &mappedRange);
        }
        vkUnmapMemory(logicalDevice, *memory);
      }

      // Attach the memory to the buffer
      VK_CHECK(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

      return VK_SUCCESS;
    }

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
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vk::Buffer* buffer, VkDeviceSize size, void* data = nullptr)
    {
      buffer->device = logicalDevice;
      buffer->pAllocator = pAllocator;

      // Create the buffer handle
      VkBufferCreateInfo bufferCInfo = vk::initializers::bufferCreateInfo(usageFlags, size);
      VK_CHECK(vkCreateBuffer(logicalDevice, &bufferCInfo, pAllocator, &buffer->buffer));

      // Create the memory to back up the buffer handle
      VkMemoryRequirements memReqs;
      VkMemoryAllocateInfo memAllocInfo = vk::initializers::memoryAllocateInfo();
      vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memReqs);
      memAllocInfo.allocationSize = memReqs.size;
      // Find a memory type index that fits the properties of this buffer
      memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
      VK_CHECK(vkAllocateMemory(logicalDevice, &memAllocInfo, pAllocator, &buffer->memory));

      buffer->alignment = memReqs.alignment;
      buffer->size = memReqs.size;
      buffer->usageFlags = usageFlags;
      buffer->memoryPropertyFlags = memoryPropertyFlags;

      // If a pointer to the buffer data has been passed in, map the buffer and copy the data into it
      if (data)
      {
        VK_CHECK(buffer->map());
        memcpy(buffer->mapped, data, size);
        buffer->unmap();
      }

      // Initialize a default descriptor that covers the while buffer size
      buffer->setupDescriptor();

      // Attach the memory to the buffer object
      return buffer->bind();
    }

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
    void copyBuffer(vk::Buffer* src, vk::Buffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr)
    {
      assert(dst->size <= src->size);
      assert(src->buffer);
      VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
      VkBufferCopy bufferCopy;
      if (copyRegion)
      {
        bufferCopy = *copyRegion;
      }
      else
      {
        bufferCopy.srcOffset = 0;
        bufferCopy.dstOffset = 0;
        bufferCopy.size = src->size;
      }

      vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1u, &bufferCopy);

      flushCommandBuffer(copyCmd, queue);
    }

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
    VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolResetFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
    {
      VkCommandPoolCreateInfo cmdPoolCInfo;
      cmdPoolCInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      cmdPoolCInfo.pNext = nullptr;
      cmdPoolCInfo.flags = createFlags;
      cmdPoolCInfo.queueFamilyIndex = queueFamilyIndex;
      VkCommandPool cmdPool;
      VK_CHECK(vkCreateCommandPool(logicalDevice, &cmdPoolCInfo, pAllocator, &cmdPool));
      return cmdPool;
    }

    /**
     * Allocate a command buffer from the command pool
     *
     * @param level     Level of the command buffer (primary or secondary)
     * @param begin     If true the command buffer will already begin recording (defaults to false)
     *
     * @return A handle to the allocated command buffer
     **/
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false)
    {
      VkCommandBufferAllocateInfo cmdBufferAllocInfo = vk::initializers::commandBufferAllocateInfo(cmdGraphicsPool, level, 1);

      VkCommandBuffer cmdBuffer;
      VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &cmdBufferAllocInfo, &cmdBuffer));

      // If requested, also open the command buffer for recording
      if (begin)
      {
        VkCommandBufferBeginInfo beginInfo = vk::initializers::commandBufferBeginInfo();
        VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
      }

      return cmdBuffer;
    }

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
    void flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, bool free = true)
    {
      assert(cmdBuffer);

      VK_CHECK(vkEndCommandBuffer(cmdBuffer));

      VkSubmitInfo submitInfo = vk::initializers::submitInfo(&cmdBuffer, 1u);

      // Create the fence to ensure that the command buffer has finished execution
      VkFenceCreateInfo fenceCInfo = vk::initializers::bufferCreateInfo(VK_FLAGS_NONE);
      VkFence fence;
      VK_CHECK(vkCreateFence(logicalDevice, &fenceCInfo, pAllocator, &fence));

      // Submit to the queue
      VK_CHECK(vkQueueSubmit(queue, 1u, &submitInfo, fence));
      // Wait for the fence to signal that command buffer has finished execution
      VK_CHECK(vkWaitForFences(logicalDevice, 1u, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

      vkDestroyFence(logicalDevice, fence, pAllocator);

      if (free)
      {
        vkFreeCommandBuffers(logicalDevice, cmdGraphicsPool, 1u, &cmdBuffer);
      }
    }

    /**
     * Check if an extension is supported by the physical device.
     *
     * @param extension     The name of the extension to check
     *
     * @return Is true if the extension is supported
     **/
    bool extensionSupported(std::string extension)
    {
      return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
    }
  };
}