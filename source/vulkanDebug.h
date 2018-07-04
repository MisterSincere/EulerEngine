/////////////////////////////////////////////////////////////////////
// Filename: vulkanDebug.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDefs.h" //< includes vulkan.h


namespace ee
{
  namespace vk
  {
    namespace debug
    {
      // Default validation layers
      extern int validationLayerCount;
      extern const char* validationLayerNames[];

      // Default debug callback
      VKAPI_ATTR VkBool32 VKAPI_CALL messageCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg,
        void* pUserData);

      // Load debug function pointer and set debug callback
      // uf callback is null, default message callback will be used
      void setupDebug(
        VkInstance instance,
        const VkAllocationCallbacks* pAllocator,
        VkDebugReportFlagsEXT flags,
        VkDebugReportCallbackEXT callback);
      // Clear debug callback
      void freeDebugCallback(
        VkInstance instance,
        const VkAllocationCallbacks* pAllocator);
    }
  }
}