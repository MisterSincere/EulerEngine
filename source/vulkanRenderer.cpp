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

    // INPUT ASSEMBLY
    {
      inputAssemblyState = vk::initializers::inputAssemblyStateCInfo();
    }

    // VIEWPORT STATE
    {
      VkViewport vp;
      vp.x = 0.0f;
      vp.y = 0.0f;
      vp.width = static_cast<float>(swapchain->extent.width);
      vp.height = static_cast<float>(swapchain->extent.height);
      vp.minDepth = 0.0f;
      vp.maxDepth = 1.0f;

      VkRect2D scissor;
      scissor.offset = { 0u, 0u };
      scissor.extent = { swapchain->extent.width , swapchain->extent.height };

      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.pNext = nullptr;
      viewportState.flags = 0;
      viewportState.viewportCount = 1u;
      viewportState.pViewports = &vp;
      viewportState.scissorCount = 1u;
      viewportState.pScissors = &scissor;
    }

    // MULTISAMPLE STATE
    {
      multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampleState.pNext = nullptr;
      multisampleState.flags = 0;
      multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampleState.sampleShadingEnable = VK_FALSE;
      multisampleState.minSampleShading = 1.0f;
      multisampleState.pSampleMask = nullptr;
      multisampleState.alphaToCoverageEnable = VK_FALSE;
      multisampleState.alphaToOneEnable = VK_FALSE;
    }

    // BLEND STATE
    {
      /**
      * if(blendEnable) {
      *     realColor.rgb = (srcColorBlendFactor * currentColor.rgb) [[[colorBlendOp]]] (dstColorBlendFactor * previousColor.rgb);
      *     realColor.a   = (srcAlphaBlendFactor * currentColor.a)   [[[colorBlendOp]]] (dstAlphaBlendFactor * previousColor.a);
      * } else {
      *      realColor = currentColor;
      * }
      * COMMON realColor.rgb = currentColor.a * currentColor.rgb + (1 - currentColor.a) * previousColor.rgb
      **/
      VkPipelineColorBlendAttachmentState colorBlendAttachment;
      colorBlendAttachment.blendEnable = VK_TRUE;
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

      // VkPipelineColorBlendStateCreateInfo
      blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      blendState.pNext = nullptr;
      blendState.flags = 0;
      blendState.logicOpEnable = VK_FALSE;
      blendState.logicOp = VK_LOGIC_OP_NO_OP;
      blendState.attachmentCount = 1;
      blendState.pAttachments = &colorBlendAttachment;
      blendState.blendConstants[0] = 0.0f;
      blendState.blendConstants[1] = 0.0f;
      blendState.blendConstants[2] = 0.0f;
      blendState.blendConstants[3] = 0.0f;
    }
  }

  VulkanRenderer::~VulkanRenderer()
  {
    if (renderPass)
    {
      vkDestroyRenderPass(swapchain->device->logicalDevice, renderPass, swapchain->device->pAllocator);
      renderPass = VK_NULL_HANDLE;
    }
    for (size_t i = 0u; i < buffers.size(); i++)
    {
      vkFreeCommandBuffers(swapchain->device->logicalDevice, swapchain->device->cmdGraphicsPool, 1u, &(buffers[i].cmdBuffer));
      vkDestroyFramebuffer(swapchain->device->logicalDevice, buffers[i].framebuffer, swapchain->device->pAllocator);
    }
    if (depthImage)
    {
      delete depthImage;
      depthImage = nullptr;
    }
  }

  void VulkanRenderer::Create()
  {
    // DEPTH IMAGE
    {
      depthImage = new vk::intern::DepthImage(swapchain);
      depthImage->Create();
    }

    // RENDER PASS
    {
      std::vector<VkAttachmentDescription> attachmentDescriptions(2);
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

      VkAttachmentReference colorReference;
      colorReference.attachment = 0;
      colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkAttachmentReference depthReference;
      depthReference.attachment = 1;
      depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpassDescription;
      subpassDescription.flags = 0;
      subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpassDescription.inputAttachmentCount = 0u;
      subpassDescription.pInputAttachments = nullptr;
      subpassDescription.colorAttachmentCount = 1u;
      subpassDescription.pColorAttachments = &colorReference;
      subpassDescription.pResolveAttachments = nullptr;
      subpassDescription.pDepthStencilAttachment = &depthReference;
      subpassDescription.preserveAttachmentCount = 0u;
      subpassDescription.pPreserveAttachments = nullptr;

      // Subpass dependencies for layout transitions
      std::vector<VkSubpassDependency> dependencies(2);

      dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[0].dstSubpass = 0;
      dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

      dependencies[1].srcSubpass = 0;
      dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

      VkRenderPassCreateInfo renderPassCInfo;
      renderPassCInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassCInfo.pNext = nullptr;
      renderPassCInfo.flags = 0;
      renderPassCInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
      renderPassCInfo.pAttachments = attachmentDescriptions.data();
      renderPassCInfo.subpassCount = 1u;
      renderPassCInfo.pSubpasses = &subpassDescription;
      renderPassCInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
      renderPassCInfo.pDependencies = dependencies.data();

      VK_CHECK(vkCreateRenderPass(swapchain->device->logicalDevice, &renderPassCInfo, swapchain->device->pAllocator, &renderPass));
    }

    // RENDER BUFFERS
    {
      buffers.resize(swapchain->buffers.size());
      for (size_t i = 0; i < swapchain->buffers.size(); i++)
      {
        // Command buffer
        buffers[i].cmdBuffer = swapchain->device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Image views
        std::vector<VkImageView> attachments;
        attachments.push_back(swapchain->buffers[i].imageView);
        attachments.push_back(depthImage->imageView);

        // Frame buffer
        VkFramebufferCreateInfo framebufferCInfo;
        framebufferCInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCInfo.pNext = nullptr;
        framebufferCInfo.flags = 0;
        framebufferCInfo.renderPass = renderPass;
        framebufferCInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCInfo.pAttachments = attachments.data();
        framebufferCInfo.width = swapchain->extent.width;
        framebufferCInfo.height = swapchain->extent.height;
        framebufferCInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(swapchain->device->logicalDevice, &framebufferCInfo, swapchain->device->pAllocator, &(buffers[i].framebuffer)));
      }
    }

  }

  void VulkanRenderer::CreateShaderModule(const char* file, VkShaderModule& shaderModule)
  {
    // Read file code in
    std::vector<char> code = vk::tools::readFile(file);

    VkShaderModuleCreateInfo cinfo;
    cinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    cinfo.pNext = nullptr;
    cinfo.flags = 0;
    cinfo.codeSize = code.size();
    cinfo.pCode = (uint32_t*)code.data();

    VK_CHECK(vkCreateShaderModule(swapchain->device->logicalDevice, &cinfo, swapchain->device->pAllocator, &shaderModule));
  }
}