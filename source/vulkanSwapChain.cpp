/////////////////////////////////////////////////////////////////////
// Filename: vulkanSwapChain.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanSwapChain.h"

#include <algorithm>

namespace vk
{

  VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
  {
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
      return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& availableFormat : availableFormats)
    {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
        return availableFormat;
      }
    }

    // No preferred available, so just go with the first one
    return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
  {
    // Store the currently best present mode
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode : availablePresentModes)
    {
      if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
        return presentMode;
      }
      else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
      {
        bestMode = presentMode;
      }
    }

    return bestMode;
  }

  VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }

  VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities)
  {
    // Preferred composite flag
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Select the first one supported of the following list
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
    };
    for (const auto& compositeAlphaFlag : compositeAlphaFlags)
    {
      if (capabilities.supportedCompositeAlpha & compositeAlphaFlag)
      {
        compositeAlpha = compositeAlphaFlag;
        break;
      }
    }

    return compositeAlpha;
  }


  VulkanSwapchain::VulkanSwapchain(vk::VulkanDevice* device, eewindow::Window* window, bool vsync)
    : device(device), window(window)
  {
    assert(device && window);

    // Get the surface details
    surfaceDetails = device->GetSurfaceDetails();


    // OPTIONS
    {
      // Format
      surfaceFormat = chooseSwapChainSurfaceFormat(surfaceDetails.formats);

      // Present mode
      presentMode = (vsync) ? VK_PRESENT_MODE_FIFO_KHR : chooseSwapChainPresentMode(surfaceDetails.presentModes);

      // Extent
      extent = chooseSwapChainExtent(surfaceDetails.capabilities, window->window);

      // Composite alpha
      compositeAlpha = chooseCompositeAlpha(surfaceDetails.capabilities);

      // Pre-Transform
      preTransform = (surfaceDetails.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfaceDetails.capabilities.currentTransform;
    }


    // Acquire the available image count we will use
    imageCount = surfaceDetails.capabilities.minImageCount + 1;
    if (surfaceDetails.capabilities.maxImageCount > 0 && imageCount > surfaceDetails.capabilities.maxImageCount)
    {
      imageCount = surfaceDetails.capabilities.maxImageCount;
    }

    // Acquire function pointer
    fpCreateSwapchainKHR = VK_FUNCTION(device->instance->instance, vkCreateSwapchainKHR);
    fpDestroySwapchainKHR = VK_FUNCTION(device->instance->instance, vkDestroySwapchainKHR);
    fpGetSwapchainImagesKHR = VK_FUNCTION(device->instance->instance, vkGetSwapchainImagesKHR);
    fpAcquireNextImageKHR = VK_FUNCTION(device->instance->instance, vkAcquireNextImageKHR);
    fpQueuePresentKHR = VK_FUNCTION(device->instance->instance, vkQueuePresentKHR);
  }

  VulkanSwapchain::~VulkanSwapchain()
  {
    for (size_t i = 0; i < buffers.size(); i++)
    {
      vkDestroyImageView(device->logicalDevice, buffers[i].imageView, device->pAllocator);
    }

    if (swapchain != VK_NULL_HANDLE)
    {
      fpDestroySwapchainKHR(device->logicalDevice, swapchain, device->pAllocator);
      swapchain = VK_NULL_HANDLE;
    }
  }

  void VulkanSwapchain::Create()
  {
    // Store old swapchain temporarily
    VkSwapchainKHR oldSwapchain = swapchain;

    // Vector to hold the graphic and present queue in case that they are different families
    uint32_t concurrentQueueFamilies[] = { device->queueFamilyIndices.graphics, device->queueFamilyIndices.present };

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = window->surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;

    // The images will attach color to the presented image
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Check if present and graphics queues are of a different family, if so use concurrent mode
    if (device->queueFamilyIndices.graphics != device->queueFamilyIndices.present)
    {
      swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapchainCreateInfo.queueFamilyIndexCount = 2u;
      swapchainCreateInfo.pQueueFamilyIndices = concurrentQueueFamilies;
    }
    else
    {
      swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapchainCreateInfo.queueFamilyIndexCount = 0u;
      swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }
    swapchainCreateInfo.preTransform = preTransform;
    swapchainCreateInfo.compositeAlpha = compositeAlpha;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = oldSwapchain;

    VK_CHECK(fpCreateSwapchainKHR(device->logicalDevice, &swapchainCreateInfo, device->pAllocator, &swapchain));

    // If an existing swap chain is re-created, destroy the old swap chain
    // This also cleans up all the presentable images
    if (oldSwapchain != VK_NULL_HANDLE)
    {
      for (uint32_t i = 0u; i < imageCount; i++)
      {
        vkDestroyImageView(device->logicalDevice, buffers[i].imageView, device->pAllocator);
      }
      fpDestroySwapchainKHR(device->logicalDevice, oldSwapchain, device->pAllocator);
    }

    // Get information about the real image count
    VK_CHECK(fpGetSwapchainImagesKHR(device->logicalDevice, swapchain, &imageCount, nullptr));

    // Get the swap chain images
    images.resize(imageCount);
    VK_CHECK(fpGetSwapchainImagesKHR(device->logicalDevice, swapchain, &imageCount, images.data()));

    // Get the swapchain buffers containing the image and imageview
    buffers.resize(imageCount);
    VkImageViewCreateInfo imageViewCInfo;
    for (uint32_t i = 0u; i < imageCount; i++)
    {
      imageViewCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      imageViewCInfo.pNext = nullptr;
      imageViewCInfo.flags = 0;
      
      buffers[i].image = images[i];
      imageViewCInfo.image = buffers[i].image;

      imageViewCInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      imageViewCInfo.format = surfaceFormat.format;
      imageViewCInfo.components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A,
      };

      imageViewCInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageViewCInfo.subresourceRange.baseMipLevel = 0u;
      imageViewCInfo.subresourceRange.levelCount = 1u;
      imageViewCInfo.subresourceRange.baseArrayLayer = 0u;
      imageViewCInfo.subresourceRange.layerCount = 1u;

      VK_CHECK(vkCreateImageView(device->logicalDevice, &imageViewCInfo, device->pAllocator, &buffers[i].imageView));
    }
  }

  VkResult VulkanSwapchain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
  {
    return fpAcquireNextImageKHR(device->logicalDevice, swapchain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
  }

  VkResult VulkanSwapchain::presentImage(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
  {
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1u;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    // Set semaphore to wait for if one was passed in
    if (waitSemaphore != VK_NULL_HANDLE)
    {
      presentInfo.waitSemaphoreCount = 1u;
      presentInfo.pWaitSemaphores = &waitSemaphore;
    }
    else
    {
      presentInfo.waitSemaphoreCount = 0u;
      presentInfo.pWaitSemaphores = nullptr;
    }

    return fpQueuePresentKHR(queue, &presentInfo);
  }


}