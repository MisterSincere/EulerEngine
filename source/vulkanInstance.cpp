/////////////////////////////////////////////////////////////////////
// Filename: vulkanInstance.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanInstance.h"

namespace vk
{

  VulkanInstance::VulkanInstance(eewindow::Window* window, std::vector<const char*> desiredLayers, std::vector<const char*> desiredExtensions)
  {
    assert(window);

    uint32_t count;

    // LAYERS
    {
      // Store supported layers
      vkEnumerateInstanceLayerProperties(&count, nullptr);
      supportedLayers.resize(count);
      vkEnumerateInstanceLayerProperties(&count, supportedLayers.data());

      // Store desired layer if they are available
      bool supported{ false };
      for (const auto& currDesiredLayer : desiredLayers)
      {
        supported = false;
        for (const auto& currSupportedLayer : supportedLayers)
        {
          // If the names are the same the layer is supported so we can stop searching
          if (strcmp(currDesiredLayer, currSupportedLayer.layerName) == 0)
          {
            supported = true;
            break;
          }
        }
        
        // Act according to if the current desired layer is supported or not
        if (supported)
        {
          enabledLayers.push_back(currDesiredLayer);
        }
        else
        {
          EEPRINT("Instance Layer: %s was requested but is not supported!\n", currDesiredLayer);
        }
      }
    }

    // EXTENSIONS
    {
      // Store supported extensions
      vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
      supportedExtensions.resize(count);
      vkEnumerateInstanceExtensionProperties(nullptr, &count, supportedExtensions.data());

      // Store desired extensions if they are available
      bool supported{ false };
      for (const auto& currDesiredExtension : desiredExtensions)
      {
        supported = false;
        for (const auto& currSupportedExtension : supportedExtensions)
        {
          // If the strings are the same the current desired extension is supported so stop searching
          if (strcmp(currDesiredExtension, currSupportedExtension.extensionName) == 0)
          {
            supported = true;
            break;
          }
        }

        // Act according to if the current desired extension is supported or not
        if (supported)
        {
          enabledExtensions.push_back(currDesiredExtension);
        }
        else
        {
          EEPRINT("Instance Extension: %s was requested but is not supported!\n", currDesiredExtension);
        }
      }

      // Add required extensions from the window (checked at window creation)
      auto windowExts = window->instanceExtensions();
      enabledExtensions.insert(enabledExtensions.end(), windowExts.begin(), windowExts.end());
    }

  }

  VulkanInstance::~VulkanInstance()
  {
    if (instance) vkDestroyInstance(instance, pAllocator);

    // Free memory
    supportedLayers.~vector();
    enabledLayers.~vector();
    supportedExtensions.~vector();
    enabledExtensions.~vector();
  }

  VkResult VulkanInstance::Create(const VkAllocationCallbacks* pAllocator)
  {
    this->pAllocator = pAllocator;

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = name;
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCInfo;
    instanceCInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCInfo.pNext = nullptr;
    instanceCInfo.flags = 0;
    instanceCInfo.pApplicationInfo = &appInfo;
    instanceCInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    instanceCInfo.ppEnabledLayerNames = enabledLayers.data();
    instanceCInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceCInfo.ppEnabledExtensionNames = enabledExtensions.data();

    return vkCreateInstance(&instanceCInfo, pAllocator, &instance);
  }

}