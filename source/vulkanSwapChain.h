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
  struct VulkanSwapchain
  {
    /* @brief A struct that holds the image and its view */
    struct SwapchainBuffer
    {
      VkImage image;
      VkImageView imageView;
    };

    /* @brief Representation of the device that will be used */
    vk::VulkanDevice* device;
    /* @brief Representation of the window we will present on */
    eewindow::Window* window;

    /* @brief Handle to the swap chain */
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

    /* @brief The format the swapchain will present in */
    VkSurfaceFormatKHR surfaceFormat;
    /* @brief The mode determining how the swapchain will be presented */
    VkPresentModeKHR presentMode;
    /* @brief Extent of the swapchain */
    VkExtent2D extent;
    /* @brief The minimal amount of images that the swapchain is holding */
    uint32_t imageCount;
    /* @brief Describes how different surfaces alpha values are composited */
    VkCompositeAlphaFlagBitsKHR compositeAlpha;
    /* @brief The pre transform of the surface */
    VkSurfaceTransformFlagBitsKHR preTransform;

    /* @brief Details of the surface we wanna present to */
    SurfaceDetails surfaceDetails;

    /* @brief List of the buffers in the swapchain holding the image and imageview of each one */
    std::vector<VkImage> images;
    std::vector<SwapchainBuffer> buffers;

    /* @brief Function pointer */
    PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
    PFN_vkQueuePresentKHR fpQueuePresentKHR;

    /**
     * Default constructor
     * 
     * @param 
     * @param 
     **/
    VulkanSwapchain(vk::VulkanDevice* device, eewindow::Window* window, bool vsync = false);

    /**
     * Default destructor
     **/
    ~VulkanSwapchain();

    /**
     * Creates / Recreates the swapchain compatible to the device and window passed in to the constructor.
     **/
    void Create();

    /**
     * Acquires the next image available in the swapchain
     *
     * @param presentCompleteSemaphore  Semaphore that is signaled when the image is ready for use (optional)
     * @param imageIndex                Pointer to the image index that will be increased if the next image could be acquired
     *
     * @note The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX
     *
     * @return VkResult of the image acquisition call
     **/
    VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);

    /**
     * Queue an image for presentation.
     *
     * @param queue         The queue that will handle the presentation
     * @param imageIndex    Index of the swapchain image to queue for representation
     * @param waitSemaphore Semaphore that is waited on before the image is presented
     *
     * @return VkResult of the queue presentation call
     **/
    VkResult presentImage(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
  };
}