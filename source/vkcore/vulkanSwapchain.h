/////////////////////////////////////////////////////////////////////
// Filename: vulkanSwapchain.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDevice.h" //& vulkanDebug.h vulkanInstance.h vulkanTools.h vulkanInitializers.h vulkan.h

namespace EE
{
	namespace vulkan
	{
		struct Swapchain
		{
			/* @brief A swapchain buffer combines the image with its view, since this is what we want */
			struct SwapchainBuffer
			{
				VkImage image;
				VkImageView imageView;
			};

			/* @brief The device that will be used */
			Device const* pDevice;
			/* @brief The window that swapchain will present on */
			EE::Window const* pWindow;

			/* @brief Handle of the created swapchain */
			VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

			/* @brief Holds some settings */
			struct {
				VkSurfaceFormatKHR surfaceFormat;
				VkPresentModeKHR presentMode;
				VkExtent2D extent;
				VkCompositeAlphaFlagBitsKHR compositeAlpha;
				VkSurfaceTransformFlagBitsKHR preTransform;
				bool vsyncEnabled;
				uint32_t imageCount;
			} settings;

			/* @brief Informations about the surface we are presenting on */
			SurfaceDetails surfaceDetails;

			/* @brief List of the buffers in the swapchain holding the image and image of each one */
			std::vector<VkImage> images;
			std::vector<SwapchainBuffer> buffers;

			/* @brief Pointer to functions of the swapchain khr extension that will be used */
			PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR{ nullptr };
			PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR{ nullptr };
			PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR{ nullptr };
			PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR{ nullptr };
			PFN_vkQueuePresentKHR fpQueuePresentKHR{ nullptr };

			/* @brief Indicates wether the swapchain was already created what is important for the recreation */
			bool isCreated{ false };

			/**
			 * Default constructor
			 *
			 * @param pDevice		Pointer to the device this swapchain will be created on
			 * @param pWindow		Pointer to the window this swapchain will present on
			 * @param vsync			Set to true to enable vsync (defaults to false)
			 **/
			Swapchain(
				EE::vulkan::Device const*	pDevice,
				EE::Window const*					pWindow,
				bool											vsync = false);

			/* Destructor */
			~Swapchain();

			/**
			 * Creates / Recreates the swapchain compatible to the pDevice and pWindow.
			 * When recreating it will automatically get the new size of the window and store it
			 * in settings.extent for you to get/check
			 **/
			void Create();

			/**
			 * Checks which of the images in the swapchain is the fastest image to be available and
			 * returns its index.
			 *
			 * @param presentCompleteSemaphore	A semaphore that will be signaled if the returned image is ready
			 * @param imageIndex								A pointer which content will be set to the image index
			 *
			 * @return The VkResult of the vulka acquire call or VK_ERROR_INITIALIZATION_FAILED if the swapchain wasnt created
			 **/
			VkResult AcquireNextImage(
				VkSemaphore presentCompleteSemaphore,
				uint32_t*		imageIndex) const;

			/**
			 * Queue an image for presentation.
			 *
			 * @param queue					The queue that will handle the presentation
			 * @param imageIndex		Index of the swapchain image to queue for presentation
			 * @param waitSemaphore	Semaphore that will be waited for before the image is presented (defaults)
			 *
			 * @return VkResult of the queue presentation call
			 **/
			VkResult PresentImage(
				VkQueue			queue,
				uint32_t		imageIndex,
				VkSemaphore waitSemaphore = VK_NULL_HANDLE) const;


			/* @brief Delete copy/move constructor/assignements */
			Swapchain(Swapchain const&) = delete;
			Swapchain(Swapchain&&) = delete;
			Swapchain& operator=(Swapchain const&) = delete;
			Swapchain& operator=(Swapchain&&) = delete;
		};
	}
}