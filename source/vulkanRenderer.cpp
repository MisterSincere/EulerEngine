/////////////////////////////////////////////////////////////////////
// Filename: vulkanRender.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanRenderer.h"

namespace vk
{

  VulkanRenderer::VulkanRenderer(vk::VulkanSwapchain* swapchain)
    : swapchain(swapchain)
  {
    assert(swapchain);
  }

  VulkanRenderer::~VulkanRenderer()
  {
    if (renderPass)
    {
      vkDestroyRenderPass(swapchain->device->logicalDevice, renderPass, swapchain->device->pAllocator);
      renderPass = VK_NULL_HANDLE;
    }
  }

  void VulkanRenderer::Create()
  {
    // Create depth image
    depthImage = new vk::intern::DepthImage(swapchain);
    depthImage->Create();

    VkAttachmentDescription* attachmentDescriptions = new VkAttachmentDescription[2];
    // Color attachment
    attachmentDescriptions[0].flags = 0;
    attachmentDescriptions[0].format = swapchain->surfaceFormat.format;
    attachmentDescriptions[0].samples = swapchain->device->sampleCount;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachmentDescriptions[1] = depthImage->DepthAttachmentDescription();
  }
}