/////////////////////////////////////////////////////////////////////
// Filename: vulkanInstance.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDebug.h"  //< vulkanDefs.h, vulkan.h

#include <vector>



namespace vk
{
  struct VulkanInstance
  {
    /* @brief Vulkan's per-application instance */
    VkInstance instance;
    /* @brief Pointer to the allocation info the create calls were made with */
    const VkAllocationCallbacks* pAllocator;

    /* @brief List of names from  the layers that are enabled on instance level */
    std::vector<const char*> enabledLayers;
    /* @brief List of names from the layers that are enabled on instance level */
    std::vector<const char*> enabledExtensions;

    const char* name{ "EulerEngine" };

    /* @brief A set of settings on instance level */
    struct {
      bool validation{ false };
    } settings;


    void CreateInstance(bool enableValidation, const VkAllocationCallbacks* = nullptr);

    void Release();
  };
}