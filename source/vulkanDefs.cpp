/////////////////////////////////////////////////////////////////////
// Filename: vulkanTools.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDefs.h"

#include "eedefs.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>

#include <fstream>


namespace vk
{
  namespace tools
  {
    std::vector<char> readFile(const char* fileName)
    {
      std::ifstream file;
      file.open(fileName, std::ios::binary | std::ios::ate);
      if (file.is_open())
      {
        size_t size = static_cast<size_t>(file.tellg());
        std::vector<char> fileBuffer(size);
        file.seekg(0);
        file.read(fileBuffer.data(), size);
        file.close();
        return fileBuffer;
      }
      else
      {
        EEPRINT("Failed to open file %s!\n", fileName);
        exitFatal("Failed to open shader file!\n");
      }
    }

    bool isStencilFormat(VkFormat format)
    {
      return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT;
    }

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

  namespace functions
  {
    VkResult EnumerateInstanceVersion(uint32_t* pApiVersion)
    {
      PFN_vkEnumerateInstanceVersion func = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
      return func(pApiVersion);
    }
  }
}
