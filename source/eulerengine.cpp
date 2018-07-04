/////////////////////////////////////////////////////////////////////
// Filename: eulerengine.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eulerengine.h"

#include "vulkanInstance.h"
#include "window.h"

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")


struct EulerComponents
{
  eewindow::Window* window;

  vk::VulkanInstance* pInstance;

  struct {
#if defined(_DEBUG) | defined(EE_DEBUG)
    bool validation{ true };
#else
    bool validation{ false };
#endif
  } settings;

  VkAllocationCallbacks* pAllocator{ nullptr };
};




///////////////////////////////////////////////////////////////////////////////
// VULKAN FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void vk_baseInitialize(EulerComponents* comp, const EEGraphicsCreateInfo* graphicsCInfo)
{
  // INSTANCE
  comp->pInstance = new vk::VulkanInstance;
  comp->pInstance->CreateInstance(comp->settings.validation, comp->pAllocator);
}


///////////////////////////////////////////////////////////////////////////////
// EE CREATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
bool eeCreateApplication(EEApplication& appOut, const EEWindowCreateInfo* windowCInfo, const EEGraphicsCreateInfo* graphicsCInfo)
{
  // Allocate memory for the window/graphics struct 
  appOut.window = new EEWindow;
  appOut.graphics = new EEGraphics;

  // Allocate memory for our components struct
  EulerComponents* comp = new EulerComponents;
  appOut.graphics->comp = comp;

  comp->window = new eewindow::Window;
  if (!comp->window->CreateWindow(appOut.window, windowCInfo, nullptr, nullptr))
  {
    EEPRINT("Failed to create window!\n");
    return false;
  }

  
  // Initialize vulkan components
  vk_baseInitialize(comp, graphicsCInfo);

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// EE UDPATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
bool eePollMessages(const EEApplication& app)
{
  EulerComponents* comp = reinterpret_cast<EulerComponents*>(app.graphics->comp);

  return comp->window->PollEvents();
}


///////////////////////////////////////////////////////////////////////////////
// EE RELEASE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void eeReleaseApplication(EEApplication* app)
{
  // Release euler components
  if (app->graphics->comp)
  {
    // Acquire right memory interpretation
    EulerComponents* comp = reinterpret_cast<EulerComponents*>(app->graphics->comp);

    // INSTANCE
    comp->pInstance->Release();
    delete comp->pInstance;

    // WINDOW
    comp->window->Release();
    delete comp->window;

    // Free memory
    delete app->graphics->comp;
    delete app->graphics;
  }

  delete app->window;
}