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

	///////////////////////////
	// FOREWARD DECLARATIONS //
	///////////////////////////
	namespace vulkan {
		struct SurfaceDetails;
	}

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

		/* @brief The surface that is representing this window for vulkan */
		VkSurfaceKHR surface{ VK_NULL_HANDLE };



		/**
		 * Default constructor: searches for requirements and allocates/initializes input values
		 **/
		Window();

		/**
		 * Destructor
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
			EEApplicationCreateInfo const& windowCInfo,
			fpEEWindowResize							 resizeMethod,
			void*													 pUserData);


		/**
		 * Creates the surface for this window
		 *
		 * @note Needs to be destroyed manually by calling ReleaseSurface
		 **/
		void CreateSurface(VkInstance instance, VkAllocationCallbacks const* pAllocator);

		/* @brief Releases the surface */
		void ReleaseSurface(VkInstance instance, VkAllocationCallbacks const* pAllocator) const;

		/* @brief Call to poll the events of the window */
		bool PollEvents();

		/**
		 * Get the supported formats, present modes and other capabilities of the surface that is used
		 * by this device alias was created for the window the this device uses
		 *
		 * @param physicalDevice	The gpu the surface details will be queried on
		 *
		 * @return	Struct describing the details of the used surface
		 **/
		vulkan::SurfaceDetails GetSurfaceDetails(VkPhysicalDevice physicalDevice) const;

		/**
		 * Checks if a gpu is compatible with the surface
		 * 
		 * @param physicalDevice	The device that will be checked
		 *
		 * @return Is true if the device is adequate
		 *
		 * @TODO:: Make the return value more modular -> not just a boolean but a number indicating the
		 *				 amount of necessities that the passed in device does not support. So that 0 indicates
		 *				 a perfect device and 1 a worse and 2 even worse and so on.
		 **/
		bool IsAdequate(VkPhysicalDevice physicalDevice) const;


		/* @brief Helper method to create the glfw window */
		bool glfw_createWindow(EEApplicationCreateInfo const&);


		/* @brief Delete copy/move constructor/assignements */
		Window(Window const&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window const&) = delete;
		Window& operator=(Window&&) = delete;
	};

}