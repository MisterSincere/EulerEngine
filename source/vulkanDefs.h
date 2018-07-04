/////////////////////////////////////////////////////////////////////
// Filename: vulkanDefs.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <vulkan/vulkan.h>
#include <stdio.h>


///////////
// MACRO //
///////////
#ifdef _DEBUG

#define VK_CHECK(f) \
{\
  VkResult res = f;\
  if(res != VK_SUCCESS) {\
    printf_s("VK_CHECK: VkResult is \"%s\" at line %d in %s\n", ee::vk::tools::vkResultToStr(res), __LINE__, __FILE__);\
  }\
}

#else

#define VK_CHECK(f)

#endif


namespace ee
{
  namespace vk
  {
    namespace tools
    {
      extern void exitFatal(const char* msg);

      extern const char* vkResultToStr(VkResult);
    }
  }
}