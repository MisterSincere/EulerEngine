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

  VulkanSwapChain::VulkanSwapChain(vk::VulkanDevice* device, eewindow::Window* window)
    : device(device)
  {
    // Pick the format for the swap chain
    /*surfaceFormat = chooseSwapChainSurfaceFormat(device->swapChainSupport.formats);
    // Pick the present mode for the swap chain
    presentMode = chooseSwapChainPresentMode(device->swapChainSupport.presentModes);
    // Pick the extent of the swap chain
    extent = chooseSwapChainExtent(device->swapChainSupport.capabilities, window->window);

    uint32_t imageCount = device->swapChainSupport.capabilities.minImageCount + 1;
    if (device->swapChainSupport.capabilities.maxImageCount > 0 && imageCount > device->swapChainSupport.capabilities.maxImageCount)
    {
      imageCount = device->swapChainSupport.capabilities.maxImageCount;
    }
    */
  }

  bool VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vsync)
  {

    return true;
  }
}