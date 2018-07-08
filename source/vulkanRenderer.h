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
    /* @brief Struct to encapsulate the swapchain data */
    vk::VulkanSwapchain* swapchain;

    /* @brief Handle of the render pass */
    VkRenderPass renderPass{ VK_NULL_HANDLE };

    /**/
    vk::intern::DepthImage* depthImage;

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