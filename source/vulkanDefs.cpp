/////////////////////////////////////////////////////////////////////
// Filename: vulkanDefs.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDefs.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>



namespace vk
{
  namespace tools
  {
    void exitFatal(const char* msg)
    {
      MessageBox(nullptr, msg, nullptr, MB_OK | MB_ICONERROR);
      assert(false);
    }

    const char* vkResultToStr(VkResult res)
    {
      switch(res) {
#define STR(val) case val: return #val;
        STR(VK_ERROR_OUT_OF_HOST_MEMORY);
        STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        STR(VK_ERROR_INITIALIZATION_FAILED);
        STR(VK_ERROR_LAYER_NOT_PRESENT);
        STR(VK_ERROR_EXTENSION_NOT_PRESENT);
        STR(VK_ERROR_FEATURE_NOT_PRESENT);
        STR(VK_ERROR_TOO_MANY_OBJECTS);
        STR(VK_ERROR_DEVICE_LOST);
        STR(VK_ERROR_INCOMPATIBLE_DRIVER);
        STR(VK_ERROR_SURFACE_LOST_KHR);
        STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
#undef STR
      default:
        return "UNKNOWN ERROR";
      }
    }
  }
}
