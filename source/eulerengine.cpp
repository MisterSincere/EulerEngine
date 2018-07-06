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

  VkSurfaceKHR surface;

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

  // In debug mode we want to have the debug layers used
#if defined(_DEBUG) | defined(EE_DEBUG)
  vk::debug::setupDebug(comp->pInstance->instance, comp->pAllocator, 0, VK_NULL_HANDLE);
#endif

  // Acquire surface
  comp->surface = comp->window->createSurface(comp->pInstance->instance, comp->pAllocator);


}

void vk_resize(GLFWwindow* window, int width, int height, void* pUserData)
{
  printf_s("Hello\n");
}


///////////////////////////////////////////////////////////////////////////////
// EE CREATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
bool eeCreateApplication(EEApplication& appOut, const EEWindowCreateInfo* windowCInfo, const EEGraphicsCreateInfo* graphicsCInfo)
{
  // Allocate memory
  appOut.window = new EEWindow;
  appOut.graphics = new EEGraphics;
  EulerComponents* comp = new EulerComponents;
  appOut.graphics->comp = comp;

  // Create the window
  comp->window = new eewindow::Window;
  if (!comp->window->CreateWindow(appOut.window, windowCInfo, vk_resize, &appOut))
  {
    EEPRINT("Failed to create window!\n");
    return false;
  }

  
  // Initialize vulkan
  vk_baseInitialize(comp, graphicsCInfo);

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// EE UDPATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
bool eePollMessages(const EEApplication& app)
{
  return (reinterpret_cast<EulerComponents*>(app.graphics->comp))->window->PollEvents();
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

    // SURFACE
    vkDestroySurfaceKHR(comp->pInstance->instance, comp->surface, comp->pAllocator);

    // DEBUG
#if defined(_DEBUG) | defined(EE_DEBUG)
    vk::debug::freeDebugCallback(comp->pInstance->instance, comp->pAllocator);
#endif

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


///////////////////////////////////////////////////////////////////////////////
// EEAPPLICATION
///////////////////////////////////////////////////////////////////////////////
bool EEApplication::KeyHit(EEKey key) const
{
  return (reinterpret_cast<EulerComponents*>(graphics->comp))->window->KeyHit(key);
}

bool EEApplication::KeyPressed(EEKey key) const
{
  return (reinterpret_cast<EulerComponents*>(graphics->comp))->window->KeyPressed(key);
}

void EEApplication::MouseMovement(double& dx, double& dy) const
{
  (reinterpret_cast<EulerComponents*>(graphics->comp))->window->MouseMovement(dx, dy);
}