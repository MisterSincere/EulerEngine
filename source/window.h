/////////////////////////////////////////////////////////////////////
// Filename: window.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "eedefs.h"

namespace eewindow
{
  /**
   * The callback method for window resizing.
   *
   * @param window    The window that was resized
   * @param width     The new width of the window
   * @param height    The new height of the window
   * @param pUserData A pointer to the user data the window was created with
   **/
  typedef void(*fpEEwindowResize)(GLFWwindow* window, int width, int height, void* pUserData);

  /**
   * The callback method for key events. 
   *
   * @param window    The window that received the event
   * @param key       The key that was pressed or released
   * @param scancode  The system-specific scancode of the key
   * @param action    GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT
   * @param mods      Bit field that described which modifier keys were held down
   * @param pUserData A pointer to the user data the window was created with
   **/
  typedef void(*fpEEkeyEvent)(GLFWwindow* window, int key, int scancode, int action, int mods, void* pUserData);

  /**
   * The callback method for new mouse positions
   * 
   * @param window    The window the movement belongs to
   * @param xpos      The new x coordinate of the mouse
   * @param ypos      The new y coordinate of the mouse
   * @param pUserData A pointer to the user data the window was created with
   **/
  typedef void(*fpEEcursorPos)(GLFWwindow* window, double xpos, double ypos, void* pUserData);

  /* @brief Holds the different callback methods */
  struct WindowCallbacks
  {
    fpEEwindowResize  windowResize;
    fpEEkeyEvent      keyEvent;
    fpEEcursorPos     cursorPos;
  };

  /* @brief A struct to hold the passed in user data and the callback functions passed in */
  struct UserData {
    const WindowCallbacks* callbacks;
    void* pUserData;
  };

  struct Window
  {
    /* @brief Pointer to the window handle of glfw */
    GLFWwindow* window;
    /* @brief The pointer to the monitor the window is created on */
    GLFWmonitor* monitor;

    // Settings of this window
    struct {
      /* @brief The current screen mode of the window (like windowed or fake fullscreen) */
      EEScreenMode screenMode;
      /* @brief The client size of the window, hence the drawable area */
      EERect32U clientSize;
      /* @brief The position oriented to the left top of the main monitor */
      EEPoint32 position;
      /* @brief Is true if the mouse will be fixed at the center and is invisible */
      bool mouseDisabled;
    } settings;

    UserData userData;

    const char* title;

    /**
     * Creates a window with glfw according to the passed in informations.
     *
     * @param winOut      A pointer to the window structure to be filled out
     * @param windowCInfo A pointer to the structure defining the different options for the window
     *
     * @return Is true if the window creation was successfull
     **/
    bool CreateWindow(
      EEWindow* winOut,
      const EEWindowCreateInfo* windowCInfo,
      const WindowCallbacks* callbacks,
      void* pUserData);

    /* @brief Call to free memory and destroy the window */
    void Release();

    /* @brief Call to poll the events of the window */
    bool PollEvents();

    /* @brief Helper method to initialize glfw. Will call glfw_initWindow automatically to create a window */
    bool glfw_initialize(
      const EEWindowCreateInfo* windowCInfo,
      const WindowCallbacks* callbacks,
      void* pUserData);

    bool glfw_initWindow(const EEWindowCreateInfo* windowCInfo);

  };
}