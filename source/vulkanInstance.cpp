/////////////////////////////////////////////////////////////////////
// Filename: vulkanInstance.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanInstance.h"


namespace vk
{
  void VulkanInstance::CreateInstance(bool enableValidation, const VkAllocationCallbacks* pAllocator)
  {
    this->pAllocator = pAllocator;
    settings.validation = enableValidation;

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = name;
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // LAYERS
    if (settings.validation)
    {
      enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    // EXTENSIONS
    enabledExtensions.push_back("VK_KHR_surface");
    if (settings.validation)
    {
      enabledExtensions.push_back("VK_EXT_debug_report");
    }

    VkInstanceCreateInfo instanceCInfo;
    instanceCInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCInfo.pNext = nullptr;
    instanceCInfo.flags = 0;
    instanceCInfo.pApplicationInfo = &appInfo;
    instanceCInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    instanceCInfo.ppEnabledLayerNames = enabledLayers.data();
    instanceCInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceCInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VK_CHECK(vkCreateInstance(&instanceCInfo, pAllocator, &instance));
  }

  void VulkanInstance::Release()
  {
    vkDestroyInstance(instance, pAllocator);

    // Free memory
    enabledLayers.~vector();
    enabledExtensions.~vector();
  }
}
