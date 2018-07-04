/////////////////////////////////////////////////////////////////////
// Filename: window.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "window.h"



namespace eewindow
{
  
  void glfw_onResize(GLFWwindow* window, int width, int height)
  {
    UserData* ud = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));

    ud->callbacks->windowResize(window, width, height, ud->pUserData);
  }

  void glfw_keyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    UserData* ud = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));

    ud->callbacks->keyEvent(window, key, scancode, action, mods, ud->pUserData);
  }

  void glfw_cursorPos(GLFWwindow* window, double xpos, double ypos)
  {
    UserData* ud = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));

    ud->callbacks->cursorPos(window, xpos, ypos, ud->pUserData);
  }
  

  bool Window::CreateWindow(EEWindow* window, const EEWindowCreateInfo* windowCInfo, const WindowCallbacks* callbacks, void* pUserData)
  {
    if (!glfw_initialize(windowCInfo, callbacks, pUserData))
    {
      EEPRINT("WINDOW CREATION FAILED!\n");
      return false;
    }

    // Store computed data in the winOut pointer
    window->clientSize = settings.clientSize;
    window->position = settings.position;
    window->screenMode = settings.screenMode;
    window->title = this->title;
    window->winHandle = this->window;

    return true;
  }

  void Window::Release()
  {
    glfwDestroyWindow(window);
    window = nullptr;
    monitor = nullptr;

    glfwTerminate();
  }

  bool Window::PollEvents()
  {
    glfwPollEvents();

    return glfwWindowShouldClose(window);
  }

  bool Window::glfw_initialize(
    const EEWindowCreateInfo* windowCInfo,
    const WindowCallbacks* callbacks,
    void* pUserData)
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

    if (callbacks)
    {
      // Set user pointer
      userData.callbacks = callbacks;
      userData.pUserData = pUserData;
      glfwSetWindowUserPointer(window, &userData);

      // Set callbacks
      glfwSetWindowSizeCallback(window, glfw_onResize);
      glfwSetKeyCallback(window, glfw_keyEvent);
      glfwSetCursorPosCallback(window, glfw_cursorPos);
    }

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