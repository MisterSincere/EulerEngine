/////////////////////////////////////////////////////////////////////
// Filename: vulkanDevice.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDevice.h"

using namespace vkee;


VulkanDevice::VulkanDevice(vkee::VulkanInstance* instance, eewindow::Window* window, const VkAllocationCallbacks* pAllocator)
{
  assert(instance && window);

  this->window = window;
  this->instance = instance;
  this->pAllocator = pAllocator;

  // Create the surface
  window->createSurface(instance->instance, pAllocator);

  // Pick the physical device
  physicalDevice = pickPhysicalDevice(instance->instance);

  // Store features, limits and properties of the physical device for later use
  // Device properties also contain limits and sparse properties
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  // Store supported device features
  vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
  // Memory properties are used for creating all kinds of buffers
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
  // Get the queue family properties
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  assert(queueFamilyCount > 0);
  queueFamilyProperties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

  uint32_t count;

  // LAYERS
  {
    // Store supported layers
    vkEnumerateDeviceLayerProperties(physicalDevice, &count, nullptr);
    supportedLayers.resize(count);
    VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &count, supportedLayers.data()));
  }

  // EXTENSIONS
  {
    // Store supported extensions
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    supportedExtensions.resize(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, supportedExtensions.data());
  }
}

VulkanDevice::~VulkanDevice()
{
  if (cmdGraphicsPool)
  {
    vkDestroyCommandPool(logicalDevice, cmdGraphicsPool, pAllocator);
  }
  if (logicalDevice)
  {
    vkDestroyDevice(logicalDevice, pAllocator);
  }
  if (window)
  {
    window->ReleaseSurface(instance->instance);
  }
}

VkResult VulkanDevice::Create(const VkPhysicalDeviceFeatures& desiredFeatures, std::vector<const char*>& additionalLayers, std::vector<const char*>& additionalExtensions, VkQueueFlags requestedQueueTypes)
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
    queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
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
  queueFamilyIndices.graphicsCount = queueFamilyProperties[queueFamilyIndices.graphics].queueCount;

  // Dedicated compute queue
  if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
  {
    queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
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
  queueFamilyIndices.computeCount = queueFamilyProperties[queueFamilyIndices.compute].queueCount;

  // Present queue
  {
    VkBool32 presentSupported{ VK_FALSE };
    for (size_t i = 0; i < queueFamilyProperties.size(); i++)
    {
      vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, static_cast<uint32_t>(i), window->surface, &presentSupported);
      if (presentSupported)
      {
        queueFamilyIndices.present = static_cast<uint32_t>(i);
        queueFamilyIndices.presentCount = queueFamilyProperties[i].queueCount;
        break;
      }
    }
    if (!presentSupported)
    {
      EEPRINT("No queue for presenting detacted!\n");
      vkee::tools::exitFatal("No queue for presenting detacted!");
    }
  }

  // Dedicated transfer queue
  if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
  {
    queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
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
  queueFamilyIndices.transferCount = queueFamilyProperties[queueFamilyIndices.transfer].queueCount;

  // LAYERS
  {
    for (const auto& currAdditionalLayer : additionalLayers)
    {
      if (layerSupported(currAdditionalLayer))
      {
        enabledLayers.push_back(currAdditionalLayer);
      }
      else
      {
        EEPRINT("Device Layer: %s was requested but is not supported!\n", currAdditionalLayer);
      }
    }
  }

  // EXTENSIONS
  {
    // Store from window required extensions (already checked)
    auto windowExts = window->deviceExtensions();
    enabledExtensions.insert(enabledExtensions.end(), windowExts.begin(), windowExts.end());

    // Add additional desired extensions if they are supported
    for (const auto& currAdditionalExtension : additionalExtensions)
    {
      if (extensionSupported(currAdditionalExtension))
      {
        enabledExtensions.push_back(currAdditionalExtension);
      }
      else
      {
        EEPRINT("Device Extension: %s was requested but is not supported!\n", currAdditionalExtension);
      }
    }
  }

  // FEATURES
  {
    //@TODO::!!!!!
  }

  // Create the logical device representation
  VkDeviceCreateInfo deviceCInfo;
  deviceCInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCInfo.pNext = nullptr;
  deviceCInfo.flags = 0;
  deviceCInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  deviceCInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
  deviceCInfo.ppEnabledLayerNames = enabledLayers.data();
  deviceCInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
  deviceCInfo.ppEnabledExtensionNames = enabledExtensions.data();
  deviceCInfo.pEnabledFeatures = &desiredFeatures;

  VkResult result = vkCreateDevice(physicalDevice, &deviceCInfo, pAllocator, &logicalDevice);

  if (result == VK_SUCCESS)
  {
    // Create the default drawing command pool
    cmdGraphicsPool = createCommandPool(queueFamilyIndices.graphics);
  }

  return result;
}

VkQueue VulkanDevice::GetQueue(VkQueueFlags queueFlag, bool present)
{
  uint32_t index;
  if ((queueFlag & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
    index = queueFamilyIndices.graphics;
  else if ((queueFlag & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
    index = queueFamilyIndices.compute;
  else if ((queueFlag & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
    index = queueFamilyIndices.transfer;
  else
  {
    EEPRINT("Invalid queue type!\n");
    vkee::tools::exitFatal("Invalid queue type!");
  }

  if (present) {
    index = queueFamilyIndices.present;
  }

  VkQueue queue;
  vkGetDeviceQueue(logicalDevice, index, 0u, &queue);
  return queue;
}

SurfaceDetails VulkanDevice::GetSurfaceDetails()
{
  SurfaceDetails details;
  uint32_t count{ 0u };

  // Get capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window->surface, &details.capabilities));

  // Get the available surface formats
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window->surface, &count, nullptr));
  details.formats.resize(count);
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window->surface, &count, details.formats.data()));

  // Get the available present modes to this surface
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, window->surface, &count, nullptr));
  details.presentModes.resize(count);
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, window->surface, &count, details.presentModes.data()));

  return details;
}

VkResult VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
{
  // Create the buffer handle
  VkBufferCreateInfo bufferCInfo = vkee::initializers::bufferCreateInfo(usageFlags, size);
  VK_CHECK(vkCreateBuffer(logicalDevice, &bufferCInfo, pAllocator, buffer));

  // Create the memory backing up the buffer handle
  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc = vkee::initializers::memoryAllocateInfo();
  vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  // Find a memory type index that fits the properties of this buffer
  memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memPropertyFlags);
  VK_CHECK(vkAllocateMemory(logicalDevice, &memAlloc, pAllocator, memory));

  // If a pointer to buffer data has been passed in, map the buffer and copy over the data
  if (data)
  {
    void* mapped{ nullptr };
    VK_CHECK(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
    memcpy(mapped, data, size);
    // If host coherency hasn't been requested, do a manual flush to make writes visible
    if ((memPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
      VkMappedMemoryRange mappedRange = vkee::initializers::mappedMemoryRange(*memory, size);
      vkFlushMappedMemoryRanges(logicalDevice, 1u, &mappedRange);
    }
    vkUnmapMemory(logicalDevice, *memory);
  }

  // Attach the memory to the buffer
  VK_CHECK(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

  return VK_SUCCESS;
}

VkResult VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vkee::Buffer* buffer, VkDeviceSize size, void* data)
{
  buffer->device = logicalDevice;
  buffer->pAllocator = pAllocator;

  // Create the buffer handle
  VkBufferCreateInfo bufferCInfo = vkee::initializers::bufferCreateInfo(usageFlags, size);
  VK_CHECK(vkCreateBuffer(logicalDevice, &bufferCInfo, pAllocator, &buffer->buffer));

  // Create the memory to back up the buffer handle
  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAllocInfo = vkee::initializers::memoryAllocateInfo();
  vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memReqs);
  memAllocInfo.allocationSize = memReqs.size;
  // Find a memory type index that fits the properties of this buffer
  memAllocInfo.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
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

void VulkanDevice::CreateAndUploadBuffer(void* data, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkBuffer* pBufferOut, VkDeviceMemory* pBufferMemoryOut)
{
  // STAGING BUFFER
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize,
    &stagingBuffer, &stagingBufferMemory, data);

  // BUFFER
  CreateBuffer(usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize,
    pBufferOut, pBufferMemoryOut);

  // COPY DATA TO THE DEVICE LOCAL BUFFER
  CopyBuffer(stagingBuffer, *pBufferOut, bufferSize);

  // DESTROY STAGING BUFFER
  vkFreeMemory(logicalDevice, stagingBufferMemory, pAllocator);
  vkDestroyBuffer(logicalDevice, stagingBuffer, pAllocator);
}

void VulkanDevice::CopyBuffer(vkee::Buffer* src, vkee::Buffer* dst, VkQueue queue, VkBufferCopy* copyRegion)
{
  assert(dst->size <= src->size);
  assert(src->buffer);
  VkCommandBuffer copyCmd = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
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

  FlushCommandBuffer(copyCmd, queue);
}

void VulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
  VkCommandBuffer copyCmd = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);

  VkBufferCopy copy;
  copy.srcOffset = 0;
  copy.dstOffset = 0;
  copy.size = bufferSize;

  vkCmdCopyBuffer(copyCmd, srcBuffer, dstBuffer, 1u, &copy);

  FlushCommandBuffer(copyCmd, GetQueue(VK_QUEUE_GRAPHICS_BIT));
}

void VulkanDevice::CopyBufferToImage(VkBuffer srcBuffer, VkImage& dstImage, uint32_t width, uint32_t height)
{
  VkCommandBuffer copyBuffer = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);

  VkBufferImageCopy copy;
  copy.bufferOffset = 0;
  copy.bufferRowLength = 0;
  copy.bufferImageHeight = 0;
  copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copy.imageSubresource.mipLevel = 0u;
  copy.imageSubresource.baseArrayLayer= 0u;
  copy.imageSubresource.layerCount = 1u;
  copy.imageOffset = { 0u, 0u, 0u };
  copy.imageExtent = { width, height };

  vkCmdCopyBufferToImage(copyBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copy);

  FlushCommandBuffer(copyBuffer, GetQueue(VK_QUEUE_GRAPHICS_BIT));
}

VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin, bool ss)
{
  VkCommandBufferAllocateInfo cmdBufferAllocInfo = vkee::initializers::commandBufferAllocateInfo(cmdGraphicsPool, level, 1);

  VkCommandBuffer cmdBuffer;
  VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &cmdBufferAllocInfo, &cmdBuffer));

  // If requested, also open the command buffer for recording
  if (begin)
  {
    VkCommandBufferBeginInfo beginInfo = vkee::initializers::commandBufferBeginInfo((ss) ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);
    VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
  }

  return cmdBuffer;
}

void VulkanDevice::FlushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, bool free)
{
  assert(cmdBuffer);

  VK_CHECK(vkEndCommandBuffer(cmdBuffer));

  VkSubmitInfo submitInfo = vkee::initializers::submitInfo(&cmdBuffer, 1u);

  // Create the fence to ensure that the command buffer has finished execution
  VkFenceCreateInfo fenceCInfo = vkee::initializers::bufferCreateInfo(VK_FLAGS_NONE);
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

void VulkanDevice::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties,
  VkImage* image, VkDeviceMemory* imageMemory)
{
  VkImageCreateInfo imageCInfo;
  imageCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCInfo.pNext = nullptr;
  imageCInfo.flags = 0;
  imageCInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCInfo.format = format;
  imageCInfo.extent = { width, height, 1 };
  imageCInfo.mipLevels = 1u;
  imageCInfo.arrayLayers = 1u;
  imageCInfo.samples = sampleCount;
  imageCInfo.tiling = tiling;
  imageCInfo.usage = usageFlags;
  imageCInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCInfo.queueFamilyIndexCount = 0u;
  imageCInfo.pQueueFamilyIndices = nullptr;
  imageCInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

  VK_CHECK(vkCreateImage(logicalDevice, &imageCInfo, pAllocator, image));

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(logicalDevice, *image, &memReqs);

  VkMemoryAllocateInfo allocInfo = vkee::initializers::memoryAllocateInfo();;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryProperties);

  VK_CHECK(vkAllocateMemory(logicalDevice, &allocInfo, pAllocator, imageMemory));

  vkBindImageMemory(logicalDevice, *image, *imageMemory, 0);
}

VkResult VulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView)
{
  VkImageViewCreateInfo imageViewCInfo;
  imageViewCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCInfo.pNext = nullptr;
  imageViewCInfo.flags = 0;
  imageViewCInfo.image = image;
  imageViewCInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCInfo.format = format;
  imageViewCInfo.components = {
    VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A
  };
  imageViewCInfo.subresourceRange.aspectMask = aspectFlags;
  imageViewCInfo.subresourceRange.baseMipLevel = 0u;
  imageViewCInfo.subresourceRange.levelCount = 1u;
  imageViewCInfo.subresourceRange.baseArrayLayer = 0u;
  imageViewCInfo.subresourceRange.layerCount = 1u;

  return vkCreateImageView(logicalDevice, &imageViewCInfo, pAllocator, imageView);
}

void VulkanDevice::ChangeImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
  // Start recording with single time commit command buffer
  VkCommandBuffer cmdBuffer = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);

  // Record an image barrier
  VkImageMemoryBarrier imageMemoryBarrier;
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.pNext = nullptr;

  VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

  if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_HOST_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else
  {
    EEPRINT("Layout transition not yet supported!");
    vkee::tools::exitFatal("Layout transition not yet supported!");
  }

  imageMemoryBarrier.oldLayout = oldLayout;
  imageMemoryBarrier.newLayout = newLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = image;

  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkee::tools::isStencilFormat(format)) imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }
  else
  {
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
  imageMemoryBarrier.subresourceRange.levelCount = 1u;
  imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
  imageMemoryBarrier.subresourceRange.layerCount = 1u;

  vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);

  FlushCommandBuffer(cmdBuffer, GetQueue(VK_QUEUE_GRAPHICS_BIT));
}

bool VulkanDevice::isFormatSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags flags)
{
  VkFormatProperties properties;
  vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

  if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & flags) == flags)
  {
    return true;
  }
  else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & flags) == flags)
  {
    return true;
  }

  return false;
}


VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolResetFlags createFlags)
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

uint32_t VulkanDevice::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
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

uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
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

bool VulkanDevice::layerSupported(const char* layer)
{
  for (size_t i = 0; i < supportedLayers.size(); i++)
  {
    if (strcmp(supportedLayers[i].layerName, layer) == 0)
      return true;
  }
  return false;
}

bool VulkanDevice::extensionSupported(const char* extension)
{
  for (size_t i = 0; i < supportedExtensions.size(); i++)
  {
    if (strcmp(supportedExtensions[i].extensionName, extension) == 0)
      return true;
  }
  return false;
}

VkPhysicalDevice VulkanDevice::pickPhysicalDevice(VkInstance instance)
{
  assert(instance != VK_NULL_HANDLE);

  // Acquire all physical devices
  uint32_t deviceCount = 0u;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0u) {
    EEPRINT("Vulkan Error: Failed to find a GPU with vulkan support!\n");
    vkee::tools::exitFatal("Vulkan Error: Failed to find a GPU with vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  VkPhysicalDevice pickedDevice{ VK_NULL_HANDLE };
  for (const auto& device : devices)
  {
    if (window->isAdequate(device))
    {
      pickedDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE)
  {
    EEPRINT("Vulkan Error: Failed to find a suitable GPU!\n");
    throw std::runtime_error("Vulkan Error: Failed to find a suitable GPU!");
  }

  return pickedDevice;
}
