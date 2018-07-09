/////////////////////////////////////////////////////////////////////
// Filename: vulkanRender.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "eeIntern.h"
#include "vulkanSwapChain.h"


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

    /* @brief Description from the renderer that is for all pipelines to use */
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineMultisampleStateCreateInfo multisampleState;
    VkPipelineColorBlendStateCreateInfo blendState;


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

    /**
     * Creates a shader module
     *
     * @param fileName        The destination of the file
     * @param shaderModuleOut Handle of the shader module that is created in here
     **/
    void CreateShaderModule(const char* fileName, VkShaderModule& shaderModuleOut);

    /**
     * Method to get the all valid input assembly state info.
     *
     * @return Valid info for the input assembly state according to this renderer
     **/
    VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCreateInfo() { return inputAssemblyState; }

    /**
     * Method to get the all valid viewport state info.
     *
     * @return Valid info for the viewport state according to this renderer
     **/
    VkPipelineViewportStateCreateInfo GetViewportStateCreateInfo() { return viewportState; }

    /**
     * Method to get the a valid multisample state info.
     *
     * @return Valid multisample info according to the current amount of samples
     **/
    VkPipelineMultisampleStateCreateInfo GetMultisampleStateCreateInfo() { return multisampleState; }

    /**
    * Method to get the a valid blend state info.
    *
    * @return Valid color blend info according to this renderer
    **/
    VkPipelineColorBlendStateCreateInfo GetColorBlendStateCreateInfo() { return blendState; }

  };
}