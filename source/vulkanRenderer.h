/////////////////////////////////////////////////////////////////////
// Filename: vulkanRenderer.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanSwapchain.h" //& vulkanDevice vulkanDebug vulkanInstance vulkanTools vulkanInitializers vulkan


namespace EE
{
	//////////////////////////
	// FORWARD DECLARATIONS //
	//////////////////////////
	struct Object;

	namespace vulkan
	{

		struct DepthImage
		{
			Swapchain const* pSwapchain;

			/* @brief Image, its memory and the view to the image */
			VkImage image;
			VkDeviceMemory imageMemory;
			VkImageView imageView;

			/* @brief Indicates wether image/imageMemory/imageView are currently created */
			bool isCreated{ false };

			/* @brief The format and boolean indicating wether its an stencil compatible format */
			/* @note Are persistent during whole instance creation */
			VkFormat format{ VK_FORMAT_UNDEFINED };
			bool isStencil;

			/* @brief Description of this image as a depth attachment */
			VkAttachmentDescription depthAttachmentDescription;
			/* @brief Depth stencil state description valid for all depth images */
			static VkPipelineDepthStencilStateCreateInfo depthStencilStateCInfo;

			DepthImage(Swapchain const* pSwapchain);
			~DepthImage();

			void Create();
			void Release();
		};

		struct Renderer
		{
			/* @brief Struct representing a render buffer by combining a framebuffer with its cmd buffer */
			struct RenderBuffer
			{
				ExecBuffer execBuffer;
				VkFramebuffer framebuffer;
			};

			/* @brief Swapchain this renderer will use to present */
			Swapchain* pSwapchain;

			/* @brief Handle of the 3d render pass */
			VkRenderPass renderPass3D{ VK_NULL_HANDLE };
			/* @brief List of the render buffers per image passed in for 3d rendering */
			std::vector<RenderBuffer> buffers3D;
			/* @brief Indicates wether the vulkan resources for 3d rendering were created or not */
			bool isCreated3D{ false };
			/* @brief Struct representing the depth image */
			DepthImage* pDepthImage{ nullptr };

			/* @brief Handle of the 2d render pass */
			VkRenderPass renderPass2D{ VK_NULL_HANDLE };
			/* @brief List of the render buffers per image passed in for 2d rendering */
			std::vector<RenderBuffer> buffers2D;
			/* @brief Indicates wether the vulkan resources for 2d rendering were created or not */
			bool isCreated2D{ false };

			/* @brief Encapsulates the needed semaphore for synchronizing the draw method */
			struct {
				VkSemaphore imageAvailable{ VK_NULL_HANDLE };
				VkSemaphore imageRendered3D;
				VkSemaphore imageRendered2D;
			} semaphores;

			/* @brief Descriptions that need to be used for all pipelines using this renderer */
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
			VkPipelineViewportStateCreateInfo viewportState;
			VkPipelineMultisampleStateCreateInfo multisampleState;
			VkPipelineColorBlendStateCreateInfo blendState;

			/* @brief Holds the settings for this renderer */
			struct {
				EESplitscreenMode splitscreen;
				VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };
			} settings;

			/**
			 * Default constructor
			 *
			 * @param pSwapchain		The swapchain that is used for presenting
			 * @param settings			Settings of this application
			 */
			Renderer(Swapchain* pSwapchain, EEApplicationCreateInfo const& settings);

			/**
			 * Destructor
			 **/
			~Renderer();

			/**
			 * Creates the 3d render pass
			 **/
			void Create3D();

			/**
			 * Creates the 2d render pass
			 **/
			void Create2D();

			/**
			 * Resizes the necessary vulkan resources to the new size
			 *
			 * @note Waits till the device is idle
			 *
			 * @param objectToDraw	Objects that should be renderer
			 *
			 * @note The passed in extent will be checked by the swapchain
			 **/
			void Resize(std::vector<Object*> const& objectsToDraw);

			/**
			 * Creates a shader module
			 * 
			 * @param fileName					Destination of the shader file
			 * @param shaderModuleOut		Handle of the shader module that was created
			 **/
			void CreateShaderModule(char const* fileName, VkShaderModule& shaderModuleOut) const;

			/**
			 * Records the draw calls of the passed in objects in the command buffers.
			 * Each swapchain image as its own command buffer
			 * 
			 * @param objectsToDraw		List of all objects that are desired to be drawn
			 **/
			void RecordDrawCommands(std::vector<Object*> const& objectsToDraw);

			/**
			 * Renders the next available image and presents it
			 **/
			void Draw();

			/**
			 * Returns if the renderer is idle
			 **/
			void WaitTillIdle();
		};
	}
}