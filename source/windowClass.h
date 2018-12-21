/////////////////////////////////////////////////////////////////////
// Filename: window.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

/////////////////
// MY INCLUDES //
/////////////////
#include "eedefs.h"

namespace EE {

	/**
   * The callback method for window resizing.
   *
   * @param window    The window that was resized
   * @param width     The new width of the window
   * @param height    The new height of the window
   * @param pUserData A pointer to the user data the window was created with
   **/
	typedef void(*fpEEWindowResize)(GLFWwindow* window, int width, int height, void* pUserData);

	struct Window {

		/* @brief A struct to hold the passed in user data and the callback functions passed in */
		struct UserData {
			fpEEWindowResize resizeCallback;
			void* pUserData;
			Window* window;
		};

		/* @brief Pointer to the window handle of glfw */
		GLFWwindow* window;
		/* @brief The pointer to the monitor the window is created on */
		GLFWmonitor* monitor;

		/* @brief List of required extensions on vulkan instance level for this window */
		std::vector<char const*> requiredInstanceExtensions;
		/* @brief List of required extensions on vulkan device level for this window*/
		std::vector<char const*> requiredDeviceExtensions;

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
			/* @brief The title of the window shown */
			const char* title;
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

		/* @brief User data that is needed in glfw callbacks */
		UserData userData;



		/**
		 * Default constructor.
		 * Searches for requirements and allocates/initializes input values.
		 **/
		Window();

		/**
		 * Default destructor
		 * @note The surface needs to be released manually by calling ReleaseSurface()
		 **/
		~Window();

		/**
		 * Creates a window with glfw according to the passed in informations.
		 * Also checks if the extensions needed to draw to such a window a supported.
		 *
		 * @param windowCInfo   A pointer to the structure defining the different options for the window
		 * @param resizeMethod  Function that will be called if the window gets resized
		 * @apram pUserData     A pointer that will be passed in to the resize method
		 *
		 * @return Is true if the window creation was successfull
		 **/
		EEBool32 Create(
			EEWindowCreateInfo const& windowCInfo,
			fpEEWindowResize					resizeMethod,
			void*											pUserData);

		/* @brief Call to poll the events of the window */
		bool PollEvents();


		/* @brief Helper method to create the glfw window */
		bool glfw_createWindow(EEWindowCreateInfo const&);
	};

}