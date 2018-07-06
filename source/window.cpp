/////////////////////////////////////////////////////////////////////
// Filename: window.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "window.h"

#include "vulkanDefs.h"

#include <assert.h>


namespace eewindow
{
  
  void glfw_onResize(GLFWwindow* window, int width, int height)
  {
    UserData* ud = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));

    if(ud->resizeCallback) ud->resizeCallback(window, width, height, ud->pUserData);
    else printf_s("RESIZE: No callback function defined!\n");
  }

  void glfw_keyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    eewindow::Window* eewindow = (reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window)))->window;

    if (action == GLFW_PRESS)
    {
      eewindow->input.keysPressed[key] = true;
      eewindow->input.keysHit[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
      eewindow->input.keysPressed[key] = false;
    }
  }

  void glfw_cursorPos(GLFWwindow* window, double xpos, double ypos)
  {
    eewindow::Window* eewindow = (reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window)))->window;

    eewindow->input.mouseXDelta = xpos - eewindow->input.mouseX;
    eewindow->input.mouseYDelta = ypos - eewindow->input.mouseY;
    eewindow->input.mouseX = xpos;
    eewindow->input.mouseY = ypos;
  }
  

  bool Window::CreateWindow(EEWindow* window, const EEWindowCreateInfo* windowCInfo, fpEEwindowResize resizeCallback, void* pUserData)
  {
    // Initialize the input values
    input.keysPressed = new bool[GLFW_KEY_LAST];
    input.keysHit = new bool[GLFW_KEY_LAST];
    memset(input.keysPressed, 0, sizeof(bool) * GLFW_KEY_LAST);
    memset(input.keysHit, 0, sizeof(bool) * GLFW_KEY_LAST);
    input.mouseXDelta = input.mouseYDelta = 0;

    // Initialize the window
    if (!glfw_initialize(windowCInfo, resizeCallback, pUserData))
    {
      EEPRINT("WINDOW CREATION FAILED!\n");
      return false;
    }

    // Set mouse position to the current one
    glfwGetCursorPos(this->window, &input.mouseX, &input.mouseY);


    // Store computed data in the winOut pointer
    window->clientSize = settings.clientSize;
    window->position = settings.position;
    window->screenMode = settings.screenMode;
    window->title = this->title;
    window->winHandle = this->window;

    return true;
  }

  VkSurfaceKHR Window::createSurface(VkInstance instance, const VkAllocationCallbacks* pAllocator)
  {
    assert(instance != VK_NULL_HANDLE);

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(instance, this->window, pAllocator, &surface));

    return surface;
  }

  void Window::Release()
  {
    // Free memory
    if (input.keysHit) {
      delete[] input.keysHit;
      input.keysHit = nullptr;
    }
    if (input.keysPressed) {
      delete[] input.keysPressed;
      input.keysPressed = nullptr;
    }

    glfwDestroyWindow(window);
    window = nullptr;
    monitor = nullptr;

    glfwTerminate();
  }

  bool Window::PollEvents()
  {
    // Reset input values
    memset(input.keysHit, 0, sizeof(bool) * GLFW_KEY_LAST);
    input.mouseXDelta = input.mouseYDelta = 0;

    glfwPollEvents();

    return glfwWindowShouldClose(window);
  }

  bool Window::KeyHit(EEKey key) const
  {
    return input.keysHit[key];
  }

  bool Window::KeyPressed(EEKey key) const
  {
    return input.keysPressed[key];
  }

  void Window::MouseMovement(double& dx, double& dy) const
  {
    dx = input.mouseXDelta;
    dy = input.mouseYDelta;
  }

  bool Window::glfw_initialize(const EEWindowCreateInfo* windowCInfo, fpEEwindowResize resizeCallback, void* pUserData)
  {
    // Call glfw's init function and hint that we are going to use vulkan
    if (!glfwInit())
    {
      EEPRINT("GLFW Error: failed to initialize GLFW (internal error)!");
      return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the windows
    if (!glfw_initWindow(windowCInfo))
    {
      return false;
    }

    // Set window user pointer
    userData.resizeCallback = resizeCallback;
    userData.pUserData = pUserData;
    userData.window = this;

    glfwSetWindowUserPointer(window, &userData);

    // Set callbacks
    glfwSetWindowSizeCallback(window, glfw_onResize);
    glfwSetKeyCallback(window, glfw_keyEvent);
    glfwSetCursorPosCallback(window, glfw_cursorPos);

    return true;
  }

  bool Window::glfw_initWindow(const EEWindowCreateInfo* cinfo)
  {
    title = cinfo->title;
    settings.screenMode = cinfo->screenMode;
    settings.position = cinfo->position;
    settings.mouseDisabled = cinfo->mouseDisabled;

    // Get information about the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Fake fullscreen video mode
    if (settings.screenMode == EE_SCREEN_MODE_FAKE_FULLSCREEN)
    {
      glfwWindowHint(GLFW_RED_BITS, mode->redBits);
      glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
      glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
      glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    // Set client size
    if (settings.screenMode == EE_SCREEN_MODE_WINDOWED)
    {
      settings.clientSize = cinfo->clientSize;
    }
    else
    {
      settings.clientSize = { static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height) };
    }

    // Set window to maximized if desired
    if (settings.screenMode == EE_SCREEN_MODE_FAKE_FULLSCREEN)
    {
      glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }
    else
    {
      glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    }

    // Create the window handle
    monitor = (settings.screenMode == EE_SCREEN_MODE_FULLSCREEN || settings.screenMode == EE_SCREEN_MODE_FAKE_FULLSCREEN) ? glfwGetPrimaryMonitor() : nullptr;
    window = glfwCreateWindow(settings.clientSize.width, settings.clientSize.height, title, monitor, nullptr);
    if (!window)
    {
      EEPRINT("GLFW ERROR: Failed to create glfw window!\n");
      return false;
    }

    // Set custom position if in windowed mode
    if (settings.screenMode == EE_SCREEN_MODE_WINDOWED)
    {
      int posX = settings.position.x, posY = settings.position.y;

      if (cinfo->flags & EE_WINDOW_FLAGS_CENTERX)
      {
        posX = (int)((mode->width - settings.clientSize.width) / 2.0f);
      }
      if (cinfo->flags & EE_WINDOW_FLAGS_CENTERY)
      {
        posX = (int)((mode->height - settings.clientSize.height) / 2.0f);
      }
      glfwSetWindowPos(window, posX, posY);
    }

    // ICON
    if (cinfo->icon)
    {
      // @TODO
    }

    if (settings.mouseDisabled)
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    return true;
  }

}