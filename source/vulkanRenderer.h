/////////////////////////////////////////////////////////////////////
// Filename: vulkanRender.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanSwapChain.h"



namespace vkee
{
  ///////////////////////////
  // FOREWARD DECLARATIONS //
  ///////////////////////////
  struct InternDepthImage;
  struct InternObject;

  struct VulkanRenderer
  {
    struct RenderBuffer
    {
      VkCommandBuffer cmdBuffer;
      VkFramebuffer framebuffer;
    };

    /* @brief Struct to encapsulate the swapchain data */
    VulkanSwapchain* swapchain;

    /* @brief Struct representing the depth image */
    InternDepthImage* depthImage;

    /* @brief Handle of the render pass */
    VkRenderPass renderPass{ VK_NULL_HANDLE };

    /* @brief Encapsulates the needed semaphores for snychronizing the draw method */
    struct {
      VkSemaphore imageAvailable;
      VkSemaphore imageRendered;
    } semaphores;

    /* @brief List of render buffers per image in swapchain passed in */
    std::vector<RenderBuffer> buffers;

    /* @brief Description from the renderer that is for all pipelines to use */
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineMultisampleStateCreateInfo multisampleState;
    VkPipelineColorBlendStateCreateInfo blendState;

    /* @brief The splitscreen mode */
    EESplitscreenMode splitscreenMode;

    /**
     * Default constructor
     *
     * @param swapchain   The swapchain struct you wanna render to
     * @param splitscreen The desired splitscreen mode
     **/
    VulkanRenderer(VulkanSwapchain* swapchain, EESplitscreenMode splitscreen);

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
     * Records the current objects passed in to the command buffers to the swapchain images
     *
     * @param objectsToDraw   List of the objects that will be recorded to the command buffers
     **/
    void RecordSwapchainCommands(const std::vector<vkee::InternObject*>& objectsToDraw);

    /**
     * Draws the next available image and presents it
     *
     *
     **/
    void Draw();

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