/////////////////////////////////////////////////////////////////////
// Filename: vulkanDebug.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDebug.h"


#include <string>
#include <sstream>


namespace ee
{
  namespace vk
  {
    namespace debug
    {
      int32_t validationLayerCount{ 1 };
      const char* validationLayerNames[] {
        "VK_LAYER_LUNARG_standard_validation"
      };

      PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback{ VK_NULL_HANDLE };
      PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback{ VK_NULL_HANDLE };
      PFN_vkDebugReportMessageEXT dbgbreakCallback{ VK_NULL_HANDLE };

      VkDebugReportCallbackEXT msgCallback;

      VKAPI_ATTR VkBool32 VKAPI_CALL messageCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg,
        void* pUserData)
      {
        // Select prefix depending on flags passed to the callback
        // Note that multiple flags may be set for a singla validation message
        std::string prefix("");

        // Error that may result in undefined behaviour
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
          prefix += "ERROR:";
        }
        // Warnings may hint at unexpected / non-spec API use
        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
          prefix += "WARNING:";
        }
        // May indicate suboptimal usage of the API
        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
          prefix += "PERFORMANCE:";
        }
        // Information that may become useful during debugging
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        {
          prefix += "INFO:";
        }
        // Diagnostic info from the vulkan layers/loaders
        // Usually not helpful in terms of API usage, but may help du debug layer and loader problems
        if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        {
          prefix += "DEBUG:";
        }

        // Create the message to display
        std::stringstream debugMessage;
        debugMessage << prefix << " [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
          fprintf(stderr, "%s\n", debugMessage.str());
        }
        else
        {
          fprintf(stdout, "%d\n", debugMessage.str());
        }

        fflush(stdout);

        // The return value of this callback controls wether the Vulkan call that caused
        // the validation message will be aborted or not
        // We return VK_FALSE because we DO NOT want Vulkan calls that cause a validation message to
        // aborted.
        // If you wanna abort these calls, return VK_TRUE and the function will
        // return VK_ERROR_VALIDATION_FAILED_EXT
        return VK_FALSE;
      }

      void setupDebug(
        VkInstance instance,
        const VkAllocationCallbacks* pAllocator,
        VkDebugReportFlagsEXT flags,
        VkDebugReportCallbackEXT callback)
      {
        CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
        DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        dbgbreakCallback = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));

        VkDebugReportCallbackCreateInfoEXT dbgCInfo;
        dbgCInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        dbgCInfo.pNext = nullptr;
        dbgCInfo.flags = flags;
        dbgCInfo.pfnCallback = messageCallback;
        dbgCInfo.pUserData = nullptr;

        VK_CHECK(CreateDebugReportCallback(instance, &dbgCInfo, pAllocator, (callback != VK_NULL_HANDLE) ? &callback : &msgCallback));
      }

      void freeDebugCallback(VkInstance instance, const VkAllocationCallbacks* pAllocator)
      {
        if (msgCallback != VK_NULL_HANDLE)
        {
          DestroyDebugReportCallback(instance, msgCallback, pAllocator);
        }
      }

    }
  }
}