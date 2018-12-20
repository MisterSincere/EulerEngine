/////////////////////////////////////////////////////////////////////
// Filename: eeInternTexture.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"

#include "stb_image.h"

using namespace vkee;



InternTexture::InternTexture(vkee::VulkanRenderer* renderer, const char* filename)
  :renderer(renderer)
{
  assert(renderer);

  // Get the primitive data of the image
  data.pixels = stbi_load(filename, &data.width, &data.height, &data.channels, STBI_rgb_alpha);
  if (!data.pixels)
  {
    EEPRINT("Image not found: %s\n", filename);
    vkee::tools::exitFatal("Failed to read image!");
  }
}

InternTexture::~InternTexture()
{
  stbi_image_free(data.pixels);

  if (isUploaded)
  {
    vkDestroySampler(renderer->swapchain->device->logicalDevice, sampler, renderer->swapchain->device->pAllocator);
    vkDestroyImageView(renderer->swapchain->device->logicalDevice, imageView, renderer->swapchain->device->pAllocator);
    vkDestroyImage(renderer->swapchain->device->logicalDevice, image, renderer->swapchain->device->pAllocator);
    vkFreeMemory(renderer->swapchain->device->logicalDevice, imageMemory, renderer->swapchain->device->pAllocator);
        
    isUploaded = false;
  }
}

void InternTexture::Upload()
{
  if (isUploaded)
  {
    EEPRINT("Texture was already uploaded! Texture upload aborted!\n");
    return;
  }

  // Compute image size
  VkDeviceSize imageSize = data.width * data.height * 4;

  // Create a staging buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  renderer->swapchain->device->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, imageSize,
    &stagingBuffer, &stagingBufferMemory, data.pixels);

  // Create the image
  renderer->swapchain->device->CreateImage(data.width, data.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &image, &imageMemory);

  // Fill image with the data from the staging buffer
  renderer->swapchain->device->ChangeImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  renderer->swapchain->device->CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(data.width), static_cast<uint32_t>(data.height));
  renderer->swapchain->device->ChangeImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // Free staging buffer
  vkDestroyBuffer(renderer->swapchain->device->logicalDevice, stagingBuffer, renderer->swapchain->device->pAllocator);
  vkFreeMemory(renderer->swapchain->device->logicalDevice, stagingBufferMemory, renderer->swapchain->device->pAllocator);

  // Create the image view
  renderer->swapchain->device->CreateImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &imageView);

  // Create the sampler
  VkSamplerCreateInfo cinfo;
  cinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  cinfo.pNext = nullptr;
  cinfo.flags = 0;
  cinfo.magFilter = VK_FILTER_LINEAR;
  cinfo.minFilter = VK_FILTER_LINEAR;
  cinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  cinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  cinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  cinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  cinfo.mipLodBias = 0.0f;
  cinfo.anisotropyEnable = VK_TRUE;
  cinfo.maxAnisotropy = 16.0f;
  cinfo.compareEnable = VK_FALSE;
  cinfo.compareOp = VK_COMPARE_OP_ALWAYS;
  cinfo.minLod = 0.0f;
  cinfo.maxLod = 0.0f;
  cinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  cinfo.unnormalizedCoordinates = VK_FALSE;

  VK_CHECK(vkCreateSampler(renderer->swapchain->device->logicalDevice, &cinfo, renderer->swapchain->device->pAllocator, &sampler));

  // We uploaded everything to vulkan handles
  isUploaded = true;
}
