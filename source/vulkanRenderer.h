/////////////////////////////////////////////////////////////////////
// Filename: vulkanRender.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "eeIntern.h"


namespace vk
{
  struct VulkanRenderer
  {
    struct RenderBuffer
    {
      VkCommandBuffer cmdBuffer;
      VkFramebuffer framebuffer;
    };

    /* @brief Struct to encapsulate the swapchain data */
    vk::VulkanSwapchain* swapchain;

    /* @brief Struct representing the depth image */
    vk::intern::DepthImage* depthImage;

    /* @brief Handle of the render pass */
    VkRenderPass renderPass{ VK_NULL_HANDLE };

    /* @brief List of render buffers per image in swapchain passed in */
    std::vector<RenderBuffer> buffers;


    /**
     * Default constructor
     *
     * @param swapchain   The swapchain struct you wanna render to
     **/
    VulkanRenderer(vk::VulkanSwapchain* swapchain);

    /**
     * Default destructor
     **/
    ~VulkanRenderer();

    
    /**
     * Creates the render pass according to the settings passed in to the constructor.
     **/
    void Create();

  };
}