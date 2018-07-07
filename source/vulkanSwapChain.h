/////////////////////////////////////////////////////////////////////
// Filename: vulkanSwapChain.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDevice.h"
#include "window.h"

#include <vector>

namespace vk
{
  struct VulkanSwapChain
  {
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    vk::VulkanDevice* device{ nullptr };


    VulkanSwapChain(vk::VulkanDevice* device, eewindow::Window* window);

    bool Create(uint32_t* width, uint32_t* height, bool vsync = false);
  };
}