/////////////////////////////////////////////////////////////////////
// Filename: vulkanInstance.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDebug.h"  //< vulkanDefs.h, vulkan.h
#include "window.h"

#include "eedefs.h"

#include <vector>
#include <stdio.h>



namespace vk
{
  struct VulkanInstance
  {
    /* @brief Vulkan's per-application instance */
    VkInstance instance{ VK_NULL_HANDLE };
    /* @brief Pointer to the allocation info the create calls were made with */
    const VkAllocationCallbacks* pAllocator;

    /* @brief List of supported layers */
    std::vector<VkLayerProperties> supportedLayers;
    /* @brief List of names from  the layers that are enabled on instance level */
    std::vector<const char*> enabledLayers;

    /* @brief List of supported extensions*/
    std::vector<VkExtensionProperties> supportedExtensions;
    /* @brief List of names from the layers that are enabled on instance level */
    std::vector<const char*> enabledExtensions;

    const char* name{ "MS:EulerEngine" };

    /* @brief Max supported api version */
    uint32_t maxApiVersion;
    /* @brief Used api version */
    uint32_t apiVersion{ VK_API_VERSION_1_0 };

    /* @brief Typecast to VkInstance */
    operator VkInstance() { return instance; }

    /**
     * Default constructor that stores the settings and checks if they are supported.
     *
     * @param window               Pointer to the window the instance needs to be created to
     * @param additionalLayers     Additional layers to the required ones from the window
     * @param additionalExtensions Additional extensions to the required ones from the window
     **/
    VulkanInstance(eewindow::Window* window, std::vector<const char*> additionalLayers, std::vector<const char*> additionalExtensions);

    /**
     * Default destructor
     **/
    ~VulkanInstance();

    /**
     * Creates the instance according to the layers and extensions desired.
     *
     * @param pAllocator  The allocation callbacks (can be nullptr for lazy alloc)
     *
     * @return The VkResult of the vkCreateInstance call
     **/
    VkResult Create(const VkAllocationCallbacks* pAllocator);

  };
}