/////////////////////////////////////////////////////////////////////
// Filename: window.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

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
  
  /* @brief A struct to hold the passed in user data and the callback functions passed in */
  struct Window;
  struct UserData {
    fpEEwindowResize resizeCallback;
    void* pUserData;
    eewindow::Window* window;
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

    // Holds input values
    struct {
      bool* keysHit;
      bool* keysPressed;
      double mouseX;
      double mouseY;
      double mouseXDelta;
      double mouseYDelta;
    } input;

    /* @brief User data to get for glfw callbacks */
    UserData userData;

    /* @brief The surface for this window */
    VkSurfaceKHR surface;
    const VkAllocationCallbacks* pAllocator;

    // The title of the window shown
    const char* title;

    /**
     * Creates a window with glfw according to the passed in informations.
     * Also checks if the extensions needed to draw to such a window a supported.
     *
     * @param winOut      A pointer to the window structure to be filled out
     * @param windowCInfo A pointer to the structure defining the different options for the window
     *
     * @return Is true if the window creation was successfull
     **/
    bool CreateWindow(
      EEWindow* winOut,
      const EEWindowCreateInfo* windowCInfo,
      fpEEwindowResize resizeMethod,
      void* pUserData);

    /**
     * Creates the surface for this window
     * @note Needs to be destroyed by calling VkSurfaceKHR
     **/
    void createSurface(VkInstance instance, const VkAllocationCallbacks* pAllocator);

    /**
     * Returns the required instance extensions for this window
     *
     * @return Vector containing the names of the required instance extensions
     **/
    std::vector<const char*> instanceExtensions();

    /**
     * Returns the required device extensions for this window
     *
     * @return Vector containing the required device extensions
     **/
    std::vector<const char*> deviceExtensions();

    /*
     * Checks if the device compatible with the surface
     *
     * @param physicalDevice    The device that should be checked
     **/
    bool isAdequate(VkPhysicalDevice physicalDevice);


    /* @brief Release the surface */
    void ReleaseSurface(VkInstance instance);

    /* @brief Call to free memory and destroy the window */
    void Release();

    /* @brief Call to poll the events of the window */
    bool PollEvents();

    /* @brief Returns true of the keys was hit once */
    bool KeyHit(EEKey key) const;

    /* @brief Returns true while the key is held down */
    bool KeyPressed(EEKey key) const;

    /* @brief the the mouse movement since last poll */
    void MouseMovement(double& dx, double& dy) const;

    /* @brief Helper method to initialize glfw. Will call glfw_initWindow automatically to create a window */
    bool glfw_initialize(
      const EEWindowCreateInfo* windowCInfo,
      fpEEwindowResize resizeMethod,
      void* pUserData);

    /* @brief Helper method to initialize the glfw window */
    bool glfw_initWindow(const EEWindowCreateInfo* windowCInfo);
  };

}