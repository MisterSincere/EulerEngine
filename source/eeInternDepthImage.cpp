/////////////////////////////////////////////////////////////////////
// Filename: eeInternDepthImage.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"

namespace vk
{
  namespace intern
  {
    DepthImage::DepthImage(vk::VulkanSwapchain* swapchain)
      : swapchain(swapchain)
    {
      assert(swapchain);
      
      // Depth format
      std::vector<VkFormat> possibleFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT
      };
      for (size_t i = 0; i < possibleFormats.size(); i++)
      {
        if (swapchain->device->isFormatSupported(possibleFormats[i], VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
          depthFormat = possibleFormats[i];
          break;
        }
      }
    }

    void DepthImage::Create()
    {
      // Check if this instance was already created
      if (isCreated)
      {
        EEPRINT("DepthImage already created!\n");
        return;
      }

      // Create the image and its memory
      swapchain->device->CreateImage(swapchain->extent.width, swapchain->extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &image, &imageMemory);

      // View to the created image
      VK_CHECK(swapchain->device->CreateImageView(image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &imageView));

      // Change the layout do a depth stencil attachment
      swapchain->device->ChangeImageLayout(image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

      isCreated = true;
    }

    void DepthImage::Release()
    {
      if (isCreated)
      {
        vkDestroyImageView(swapchain->device->logicalDevice, imageView, swapchain->device->pAllocator);
        vkDestroyImage(swapchain->device->logicalDevice, image, swapchain->device->pAllocator);
        vkFreeMemory(swapchain->device->logicalDevice, imageMemory, swapchain->device->pAllocator);

        isCreated = false;
      }
    }

    VkAttachmentDescription DepthImage::DepthAttachmentDescription()
    {
      VkAttachmentDescription depthAttachment;
      depthAttachment.flags = 0;
      depthAttachment.format = depthFormat;
      depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      return depthAttachment;
    }

    VkPipelineDepthStencilStateCreateInfo DepthImage::DepthStencilStateCInfoOpaque()
    {
      VkPipelineDepthStencilStateCreateInfo info;
      info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      info.pNext = nullptr;
      info.flags = 0;
      info.depthTestEnable = VK_TRUE;
      info.depthWriteEnable = VK_TRUE;
      info.depthCompareOp = VK_COMPARE_OP_LESS; //< Less is nearer
      info.depthBoundsTestEnable = VK_FALSE;
      info.stencilTestEnable = VK_FALSE;
      info.front = {};
      info.back = {};
      info.minDepthBounds = 0.0f;
      info.maxDepthBounds = 1.0f;
      return info;
    }
  }
}