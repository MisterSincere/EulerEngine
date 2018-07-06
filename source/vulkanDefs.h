/////////////////////////////////////////////////////////////////////
// Filename: vulkanDefs.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <stdio.h>

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
  }\
}

#else

#define VK_CHECK(f)

#endif

// Define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000


namespace vk
{
  namespace tools
  {
    extern void exitFatal(const char* msg);

    extern const char* vkResultToStr(VkResult);
  }
}