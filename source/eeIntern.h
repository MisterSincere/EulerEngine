/////////////////////////////////////////////////////////////////////
// Filename: eeIntern.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanSwapChain.h"

namespace vk
{
  namespace intern
  {
    //---------------------------------------------------------------------------------------
    // EEInternDepthImage
    //---------------------------------------------------------------------------------------
    struct DepthImage
    {
      /* @brief Encapsulates the swapchain this depth image is for */
      vk::VulkanSwapchain* swapchain;

      /* @brief The image buffer */
      VkImage image;
      /* @brief Memory backing up the image buffer */
      VkDeviceMemory imageMemory;
      /* @brief View to the image buffer */
      VkImageView imageView;

      /* @brief The depth format that is used */
      VkFormat depthFormat;

      /* @brief Is true if creation was successfull and release has not yet been called */
      bool isCreated{ false };

      /**
       * Default constructor, checks for settings
       *
       * @param swapchain   The swapchain the depth image is created for
       **/
      DepthImage(vk::VulkanSwapchain* swapchain);

      /**
       * Default constructor just calls the release method.
       **/
      ~DepthImage() { Release(); }

      /**
       *
       **/
      void Create();

      /**
       *
       **/
      void Release();

      /**
       * Returns the attachment description for a depth image with the format
       * depending on the swapchain this instance was created with.
       **/
      VkAttachmentDescription DepthAttachmentDescription();

      /**
       *
       **/
      static VkPipelineDepthStencilStateCreateInfo DepthStencilStateCInfoOpaque();
    };
  }
}