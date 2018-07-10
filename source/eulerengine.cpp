/////////////////////////////////////////////////////////////////////
// Filename: eulerengine.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eulerengine.h"

#include "eeIntern.h"

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")

#define GET_COMP(app) EulerComponents* comp = reinterpret_cast<EulerComponents*>(app.graphics->comp);



struct EulerComponents
{
  eewindow::Window* window;
  vk::VulkanInstance* instance;
  vk::debug::Debug* debug;
  vk::VulkanDevice* device;
  vk::VulkanSwapchain* swapchain;
  vk::VulkanRenderer* renderer;

  std::vector<vk::intern::Object*> currentObjects;
  std::vector<uint32_t*> iCurrentObjects;
  std::vector<vk::intern::Shader*> currentShader;
  std::vector<uint32_t*> iCurrentShader;
  std::vector<vk::intern::Texture*> currentTextures;
  std::vector<uint32_t*> iCurrentTextures;
  std::vector<vk::intern::Mesh*> currentMeshes;
  std::vector<uint32_t*> iCurrentMeshes;

  struct {
#if defined(_DEBUG) | defined(EE_DEBUG)
    bool validation{ true };
#else
    bool validation{ false };
#endif
  } settings;

  VkAllocationCallbacks* pAllocator{ nullptr };

  /* @brief Is true if eei create method was called but not the finish method yet */
  bool eeiFinished{ false };
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

void vk_renderer(EulerComponents* comp, EESplitscreenMode splitscreen)
{
  comp->renderer = new vk::VulkanRenderer(comp->swapchain, splitscreen);

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
  vk_renderer(comp, graphicsCInfo->splitscreen);
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

EETexture eeiCreateTexture(EEApplication& app, const char* file)
{
  GET_COMP(app);

  EEInvariant(comp->currentTextures.size() == comp->iCurrentTextures.size());

  // Push back new texture handle
  comp->currentTextures.push_back(new vk::intern::Texture(comp->renderer, file));
  comp->currentTextures[comp->currentTextures.size() - 1]->Upload();

  // Push back address of index to the new textures
  comp->iCurrentTextures.push_back(new uint32_t(static_cast<uint32_t>(comp->currentTextures.size() - 1)));

  EEInvariant(comp->currentTextures.size() == comp->iCurrentTextures.size());

  return { comp->iCurrentTextures[comp->iCurrentTextures.size() - 1] };
}

EEShader eeiCreateShader(EEApplication& app, const EEShaderCreateInfo& shaderCInfo)
{
  GET_COMP(app);

  EEInvariant(comp->currentShader.size() == comp->iCurrentShader.size());

  // Push back shader
  comp->currentShader.push_back(new vk::intern::Shader(comp->renderer, shaderCInfo));
  comp->currentShader[comp->currentShader.size() - 1]->Create();

  // Push back address of index to the new shader
  comp->iCurrentShader.push_back(new uint32_t(static_cast<uint32_t>(comp->currentShader.size() - 1)));

  EEInvariant(comp->currentShader.size() == comp->iCurrentShader.size());

  return { comp->iCurrentShader[comp->iCurrentShader.size() - 1] };
}

EEObject eeiCreateObject(EEApplication& app, EEShader shader, EEMesh mesh, EESplitscreen ss)
{
  GET_COMP(app);

  EEInvariant(comp->currentObjects.size() == comp->iCurrentObjects.size());

  // Push back object
  comp->currentObjects.push_back(new vk::intern::Object(comp->currentMeshes[*mesh], comp->currentShader[*shader]));

  // Push back address of index to the new object
  comp->iCurrentObjects.push_back(new uint32_t(static_cast<uint32_t>(comp->currentObjects.size() - 1)));

  EEInvariant(comp->currentObjects.size() == comp->iCurrentObjects.size());

  return { comp->iCurrentObjects[comp->iCurrentObjects.size() - 1] };
}

void eeFinishCreation(const EEApplication& app)
{
  GET_COMP(app);
  comp->renderer->RecordSwapchainCommands(comp->currentObjects);
  comp->eeiFinished = true;
}


///////////////////////////////////////////////////////////////////////////////
// EE UDPATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
bool eePollMessages(const EEApplication& app)
{
  return (reinterpret_cast<EulerComponents*>(app.graphics->comp))->window->PollEvents();
}

void eeUpdateUniformBuffer(const EEApplication& app, EEObject object, void* data, int binding)
{
  EulerComponents* comp = (EulerComponents*)app.graphics->comp;

  comp->currentObjects[*object]->UpdateUniformBuffer(data, binding);
}

void eeDrawFrame(const EEApplication& app)
{
  if (!app.graphics || !app.graphics->comp)
  {
    EEPRINT("Application struct invalid. No draw call possible!\n");
    vk::tools::exitFatal("Application struct invalid. No draw call possible!\n");
  }
  GET_COMP(app);
  if (!comp->eeiFinished)
  {
    EEPRINT("Object creation calls were not finished yet. Please call eeFinishCreation to fix this!\n");
    vk::tools::exitFatal("Object creation calls were not finished yet. Please call eeFinishCreation to fix this!\n");
  }

  comp->renderer->Draw();
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