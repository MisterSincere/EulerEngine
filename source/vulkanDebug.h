/////////////////////////////////////////////////////////////////////
// Filename: vulkanDebug.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDefs.h" //< includes vulkan.h



namespace vkee
{
  namespace debug
  {
    struct Debug
    {
      /* @brief The instance of the application */
      VkInstance instance;
      /* @brief Pointer to the allocation callbacks that will be used */
      const VkAllocationCallbacks* pAllocator;

      /* @brief Function to create the debug callback handle */
      PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT;
      /* @brief Function to destroy the debug callback handle */
      PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT;

      /* @brief Handle for the debug report extension */
      VkDebugReportCallbackEXT debugReport{ VK_NULL_HANDLE };

      /**
       * Default constructor
       *
       * @param instance    The instance the debug tools will be created for
       * @param pAllocator  Pointer to the allocation callbacks
       **/
      Debug::Debug(VkInstance instance, const VkAllocationCallbacks* pAllocator);

      /**
       * Default destructor
       **/
      Debug::~Debug();

      /**
       * Creates the debug handle
       **/
      void setupDebug();
    };
  }
}
