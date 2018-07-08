/////////////////////////////////////////////////////////////////////
// Filename: vulkanDebug.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDebug.h"


#include <string>
#include <sstream>


namespace vk
{
  namespace debug
  {
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
      size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
    {
      printf("%s\n", msg);
      return VK_FALSE;
    }

    Debug::Debug(VkInstance instance, const VkAllocationCallbacks* pAllocator)
      :instance(instance), pAllocator(pAllocator)
    {
      assert(instance);

      // Acquire function pointers
      fpCreateDebugReportCallbackEXT = VK_FUNCTION(instance, vkCreateDebugReportCallbackEXT);
      fpDestroyDebugReportCallbackEXT = VK_FUNCTION(instance, vkDestroyDebugReportCallbackEXT);
    }

    Debug::~Debug()
    {
      if (debugReport)
      {
        fpDestroyDebugReportCallbackEXT(instance, debugReport, pAllocator);
      }
    }

    void Debug::setupDebug()
    {
      VkDebugReportCallbackCreateInfoEXT createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
      createInfo.pNext = nullptr;
      createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
      createInfo.pfnCallback = debugCallback;
      createInfo.pUserData = nullptr;

      VK_CHECK(fpCreateDebugReportCallbackEXT(instance, &createInfo, pAllocator, &debugReport));
    }
  }
}