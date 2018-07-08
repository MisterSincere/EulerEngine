/////////////////////////////////////////////////////////////////////
// Filename: vulkanTools.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <stdio.h>
#include <assert.h>
#include <vector>

#include "vulkanInitializers.h" //< includes vulkan.h


///////////
// MACRO //
///////////
#ifdef _DEBUG

#define VK_CHECK(f) \
{\
  VkResult res = f;\
  if(res != VK_SUCCESS) {\
    printf_s("VK_CHECK: VkResult is \"%s\" at line %d in %s\n", vk::tools::vkResultToStr(res), __LINE__, __FILE__);\
    assert(res == VK_SUCCESS);\
  }\
}

#else

#define VK_CHECK(f)

#endif

// Define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

/* @brief Gets the function pointer of name and calls it with the variadic arguments passed in */
#define VK_FUNCTION_CALL(instance, name, ...) (reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name)))(__VA_ARGS__);

/* @brief Just gets the function with the name passed in */
#define VK_FUNCTION(instance, name) reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name));

namespace vk
{
  struct SurfaceDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  namespace tools
  {
    extern bool isStencilFormat(VkFormat format);

    extern void exitFatal(const char* msg);

    extern const char* vkResultToStr(VkResult);
  }
}