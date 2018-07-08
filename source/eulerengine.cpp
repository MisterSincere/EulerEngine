/////////////////////////////////////////////////////////////////////
// Filename: eulerengine.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eulerengine.h"

#include "vulkanRenderer.h"

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")


struct EulerComponents
{
  eewindow::Window* window;
  vk::VulkanInstance* instance;
  vk::debug::Debug* debug;
  vk::VulkanDevice* device;
  vk::VulkanSwapchain* swapchain;
  vk::VulkanRenderer* renderer;

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
void vk_instance(EulerComponents* comp)
{
  // LAYERS
  std::vector<const char*> instanceLayer;
  {
    // If validation is enabled active the lunarg validation layer
    if (comp->settings.validation)
    {
      instanceLayer.push_back("VK_LAYER_LUNARG_standard_validation");
    }
  }

  // EXTENSIONS
  std::vector<const char*> instanceExtensions;
  {
    // If validation is enabled activate debug report extension
    if (comp->settings.validation)
    {
      instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
  }

  comp->instance = new vk::VulkanInstance(comp->window, instanceLayer, instanceExtensions);

  VK_CHECK(comp->instance->Create(comp->pAllocator));
}

void vk_device(EulerComponents* comp)
{
  std::vector<const char*> extensions(0);
  std::vector<const char*> layers(0);

  VkPhysicalDeviceFeatures enabledFeatures{0};

  // Create the device handle
  comp->device = new vk::VulkanDevice(comp->instance, comp->window, comp->pAllocator);

  VK_CHECK(comp->device->Create(enabledFeatures, layers, extensions));
}

void vk_swapChain(EulerComponents* comp)
{
  comp->swapchain = new vk::VulkanSwapchain(comp->device, comp->window);

  comp->swapchain->Create();
}

void vk_renderer(EulerComponents* comp)
{
  comp->renderer = new vk::VulkanRenderer(comp->swapchain);

  comp->renderer->Create();
}

void vk_baseInitialize(EulerComponents* comp, const EEGraphicsCreateInfo* graphicsCInfo)
{
  // INSTANCE
  vk_instance(comp);

  printf("Max API Version: %d.%d.%d\n", VK_VERSION_MAJOR(comp->instance->maxApiVersion), VK_VERSION_MINOR(comp->instance->maxApiVersion), VK_VERSION_PATCH(comp->instance->maxApiVersion));
  printf("Used API Version: %d.%d.%d\n", VK_VERSION_MAJOR(comp->instance->apiVersion), VK_VERSION_MINOR(comp->instance->apiVersion), VK_VERSION_PATCH(comp->instance->apiVersion));


  if (comp->settings.validation)
  {
    comp->debug = new vk::debug::Debug(comp->instance->instance, comp->pAllocator);
    comp->debug->setupDebug();
  }

  // DEVICE
  vk_device(comp);

  // SWAPCHAIN
  vk_swapChain(comp);

  // RENDERER
  vk_renderer(comp);
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

    // RENDERER
    delete comp->renderer;

    // SWAPCHAIN
    delete comp->swapchain;

    // DEVICE
    delete comp->device;

    // DEBUG
    if (comp->settings.validation)
    {
      delete comp->debug;
    }

    // INSTANCE
    delete comp->instance;

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