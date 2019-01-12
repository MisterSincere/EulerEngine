/////////////////////////////////////////////////////////////////////
// Filename: vulkanRenderer.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanRenderer.h"

#include <cassert>

#include "vulkanObject.h"

using namespace EE;

#define EEDEVICE pSwapchain->pDevice
#define LDEVICE *(EEDEVICE)
#define ALLOCATOR EEDEVICE->pAllocator


//-------------------------------------------------------------------
// DepthImage
//-------------------------------------------------------------------
VkPipelineDepthStencilStateCreateInfo vulkan::DepthImage::depthStencilStateCInfo = {
	/*sType*/									VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	/*pNext*/									nullptr,
	/*flags*/									0,
	/*depthTestEnable*/				VK_TRUE,
	/*depthWriteEnable*/			VK_TRUE,
	/*depthCompareOp*/				VK_COMPARE_OP_LESS, //< Less is nearer
	/*depthBoundsTestEnable*/ VK_FALSE,
	/*stencilTestEnable*/			VK_FALSE,
	/*front*/									{},
	/*back*/									{},
	/*minDepthBounds*/				0.0f,
	/*maxDepthBounds*/				1.0f
};

vulkan::DepthImage::DepthImage(Swapchain const* pSwapchain)
	: pSwapchain(pSwapchain)
{
	assert(pSwapchain);

	// Query formats for the right one
	std::vector<VkFormat> possibleFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT
	};
	for (size_t i = 0u; i < possibleFormats.size(); i++) {
		if (EEDEVICE->IsFormatSupported(possibleFormats[i], VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
			format = possibleFormats[i];
			break;
		}
	}
	if (format == VK_FORMAT_UNDEFINED) {
		EE_PRINT("[RENDERER3D] Depth image has no compatible depth format!\n");
		EE::tools::exitFatal("Incompatible hardware!\n");
	}
	isStencil = tools::isStencilFormat(format);

	// Fill out depth attachment description for this depth image
	depthAttachmentDescription.flags = 0;
	depthAttachmentDescription.format = format;
	depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

vulkan::DepthImage::~DepthImage()
{
	Release();
}

void vulkan::DepthImage::Create()
{
	if (isCreated) {
		Release();
	}
	// Create image
	VkImageCreateInfo imageCInfo;
	imageCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCInfo.pNext = nullptr;
	imageCInfo.flags = 0;
	imageCInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCInfo.format = format;
	imageCInfo.extent = { pSwapchain->settings.extent.width, pSwapchain->settings.extent.height, 1u };
	imageCInfo.mipLevels = 1u;
	imageCInfo.arrayLayers = 1u;
	imageCInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCInfo.queueFamilyIndexCount = 0u;
	imageCInfo.pQueueFamilyIndices = nullptr;
	imageCInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VK_CHECK(vkCreateImage(LDEVICE, &imageCInfo, ALLOCATOR, &image));

	// Allocate memory
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(LDEVICE, image, &memReqs);
	VkMemoryAllocateInfo allocInfo = initializers::memoryAllocateInfo();
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = EEDEVICE->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(LDEVICE, &allocInfo, ALLOCATOR, &imageMemory));

	// Bind image to memory
	VK_CHECK(vkBindImageMemory(LDEVICE, image, imageMemory, 0u));

	// Create the view to the depth image
	VkImageViewCreateInfo imageViewCInfo;
	imageViewCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCInfo.pNext = nullptr;
	imageViewCInfo.flags = 0;
	imageViewCInfo.image = image;
	imageViewCInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCInfo.format = format;
	imageViewCInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A,
	};
	imageViewCInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCInfo.subresourceRange.baseMipLevel = 0u;
	imageViewCInfo.subresourceRange.levelCount = 1u;
	imageViewCInfo.subresourceRange.baseArrayLayer = 0u;
	imageViewCInfo.subresourceRange.layerCount = 1u;
	VK_CHECK(vkCreateImageView(LDEVICE, &imageViewCInfo, ALLOCATOR, &imageView));

	// Convert image to an depth stencil attachment
	ExecBuffer execBuffer(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);

	VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (tools::isStencilFormat(format)) aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	tools::imageBarrier(execBuffer, image, aspectFlags, 1u, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

	execBuffer.EndRecording();
	execBuffer.Execute();

	isCreated = true;
}

void vulkan::DepthImage::Release()
{
	if (isCreated) {
		vkDestroyImageView(LDEVICE, imageView, ALLOCATOR);
		vkFreeMemory(LDEVICE, imageMemory, ALLOCATOR);
		vkDestroyImage(LDEVICE, image, ALLOCATOR);

		isCreated = false;
	}
}



//-------------------------------------------------------------------
// Renderer
//-------------------------------------------------------------------
vulkan::Renderer::Renderer(Swapchain* pSwapchain, EEApplicationCreateInfo const& settings)
	: pSwapchain(pSwapchain)
{
	assert(pSwapchain);

	// Store settings
	this->settings.splitscreen = settings.splitscreen;

	// Input assembly state info
	inputAssemblyState = initializers::inputAssemblyStateCInfo();

	// Viewport description
	{
		VkViewport vp;
		vp.x = 0.0f;
		vp.y = 0.0f;
		vp.width = float(pSwapchain->settings.extent.width);
		vp.height = float(pSwapchain->settings.extent.height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset = { 0u, 0u };
		scissor.extent = { pSwapchain->settings.extent.width , pSwapchain->settings.extent.height };

		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.flags = 0;
		viewportState.viewportCount = 1u;
		viewportState.pViewports = &vp;
		viewportState.scissorCount = 1u;
		viewportState.pScissors = &scissor;
	}

	// Multisample state
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pNext = nullptr;
	multisampleState.flags = 0;
	multisampleState.rasterizationSamples = this->settings.sampleCount;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	// Blend state
	/**
	 * if(blendEnable) {
	 *	 realColor.rgb = (srcColorBlendFactor * currentColor.rgb) [[colorBlendOp]] (dstColorBlendFactor * previousColor.rgb);
	 *	 realColor.a = (srcAlphaBlendFactor * currentColor.a) [[colorBlendOp]] (dstAlphaBlendFactor * previousColor.a);
	 * } else {
	 *	 realColor = currentColor;
	 * }
	 * COMMON: realColor.rgb = currentColor.a * currentColor.rgb + (1 - currentColor.a) * previousColor.rgb
	 **/
	static VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
																			| VK_COLOR_COMPONENT_G_BIT
																			| VK_COLOR_COMPONENT_B_BIT
																			| VK_COLOR_COMPONENT_A_BIT;
 
	blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendState.pNext = nullptr;
	blendState.flags = 0;
	blendState.logicOpEnable = VK_FALSE;
	blendState.logicOp = VK_LOGIC_OP_NO_OP;
	blendState.attachmentCount = 1u;
	blendState.pAttachments = &colorBlendAttachment;
	blendState.blendConstants[0] = 0.0f;
	blendState.blendConstants[1] = 0.0f;
	blendState.blendConstants[2] = 0.0f;
	blendState.blendConstants[3] = 0.0f;


	// Initialize buffer lists for both to the desired size no matter if needed
	buffers2D.resize(pSwapchain->buffers.size());
	buffers3D.resize(pSwapchain->buffers.size());
}

vulkan::Renderer::~Renderer()
{
	// 3D Renderer
	if (isCreated3D) {
		vkDestroyRenderPass(LDEVICE, renderPass3D, ALLOCATOR);

		for (size_t i = 0u; i < buffers3D.size(); i++) {
			vkDestroyFramebuffer(LDEVICE, buffers3D[i].framebuffer, ALLOCATOR);
		}

		if (pDepthImage) delete pDepthImage;

		vkDestroySemaphore(LDEVICE, semaphores.imageRendered3D, ALLOCATOR);

		isCreated3D = false;
	}


	// 2D Renderer
	if (isCreated2D) {
		vkDestroyRenderPass(LDEVICE, renderPass2D, ALLOCATOR);

		for (size_t i = 0u; i < buffers2D.size(); i++) {
			vkDestroyFramebuffer(LDEVICE, buffers2D[i].framebuffer, ALLOCATOR);
		}

		vkDestroySemaphore(LDEVICE, semaphores.imageRendered2D, ALLOCATOR);

		isCreated2D = false;
	}

	vkDestroySemaphore(LDEVICE, semaphores.imageAvailable, ALLOCATOR);
}

void vulkan::Renderer::Create3D()
{
	if (isCreated3D) return;
	if (isCreated2D) {
		EE_PRINT("[RENDERER] Please create 3d renderer before the 2d renderer if you wanna use both!\n");
		return;
	}

	// Allocate depthimage if it wasnt
	pDepthImage = new DepthImage(pSwapchain);
	// Creates/Recreates the pDepthImage
	pDepthImage->Create();

	// Create the render pass
	{
		// Set up the attachment the this render pass will get
		std::vector<VkAttachmentDescription> attachmentDescriptions(2);
		// Color attachment
		attachmentDescriptions[0].flags = 0;
		attachmentDescriptions[0].format = pSwapchain->settings.surfaceFormat.format;
		attachmentDescriptions[0].samples = settings.sampleCount;
		attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachmentDescriptions[1] = pDepthImage->depthAttachmentDescription;

		// Reference to these attachments
		VkAttachmentReference colorReference;
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthReference;
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Describe our subpass
		std::vector<VkSubpassDescription> subpasses(1);
		subpasses[0].flags = 0;
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].inputAttachmentCount = 0u;
		subpasses[0].pInputAttachments = nullptr;
		subpasses[0].colorAttachmentCount = 1u;
		subpasses[0].pColorAttachments = &colorReference;
		subpasses[0].pResolveAttachments = nullptr;
		subpasses[0].pDepthStencilAttachment = &depthReference;
		subpasses[0].preserveAttachmentCount = 0u;
		subpasses[0].pPreserveAttachments = nullptr;

		// Subpass dependencies
		std::vector<VkSubpassDependency> dependencies(2);
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the 3d render pass
		VkRenderPassCreateInfo renderPassCInfo;
		renderPassCInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCInfo.pNext = nullptr;
		renderPassCInfo.flags = 0;
		renderPassCInfo.attachmentCount = uint32_t(attachmentDescriptions.size());
		renderPassCInfo.pAttachments = attachmentDescriptions.data();
		renderPassCInfo.subpassCount = uint32_t(subpasses.size());
		renderPassCInfo.pSubpasses = subpasses.data();
		renderPassCInfo.dependencyCount = uint32_t(dependencies.size());
		renderPassCInfo.pDependencies = dependencies.data();

		VK_CHECK(vkCreateRenderPass(LDEVICE, &renderPassCInfo, ALLOCATOR, &renderPass3D));

		// Free memory
		attachmentDescriptions.~vector();
		subpasses.~vector();
		dependencies.~vector();
	}

	// Create render buffers
	{
		for (size_t i = 0u; i < buffers3D.size(); i++) {
			// Create the command buffer
			buffers3D[i].execBuffer.Create(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			// Now set tjhe actual attachment resources for the framebuffer to use
			std::vector<VkImageView> attachments = {
				pSwapchain->buffers[i].imageView,
				pDepthImage->imageView
			};

			// Create the frame buffer of this swapchain buffer
			VkFramebufferCreateInfo framebufferCInfo;
			framebufferCInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCInfo.pNext = nullptr;
			framebufferCInfo.flags = 0;
			framebufferCInfo.renderPass = renderPass3D;
			framebufferCInfo.attachmentCount = uint32_t(attachments.size());
			framebufferCInfo.pAttachments = attachments.data();
			framebufferCInfo.width = pSwapchain->settings.extent.width;
			framebufferCInfo.height = pSwapchain->settings.extent.height;
			framebufferCInfo.layers = 1u;

			VK_CHECK(vkCreateFramebuffer(LDEVICE, &framebufferCInfo, ALLOCATOR, &(buffers3D[i].framebuffer)));
		}
	}

	// Create 3d render specific semaphore
	VkSemaphoreCreateInfo semCInfo = initializers::semaphoreCreateInfo();
	if (semaphores.imageRendered3D == VK_NULL_HANDLE) {
		VK_CHECK(vkCreateSemaphore(LDEVICE, &semCInfo, ALLOCATOR, &semaphores.imageRendered3D));
	}

	// Create semaphore that is needed for 2d and 3d if it wasnt already created with Create2D method
	if (semaphores.imageAvailable == VK_NULL_HANDLE) {
		VK_CHECK(vkCreateSemaphore(LDEVICE, &semCInfo, ALLOCATOR, &semaphores.imageAvailable));
	}

	isCreated3D = true;
}

void vulkan::Renderer::Create2D()
{
	if (isCreated2D) return;

	// RENDER PASS
	{
		std::vector<VkAttachmentDescription> attachments(1);
		// Color attachment
		attachments[0].flags = 0;
		attachments[0].format = pSwapchain->settings.surfaceFormat.format;
		attachments[0].samples = settings.sampleCount;
		attachments[0].loadOp = (isCreated3D) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR; // Load because we do want to render on top for an interface
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = (isCreated3D) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Reference to the color attachment
		VkAttachmentReference colorReference;
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Describe our subpass
		std::vector<VkSubpassDescription> subpasses(1);
		subpasses[0].flags = 0;
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].inputAttachmentCount = 0u;
		subpasses[0].pInputAttachments = nullptr;
		subpasses[0].colorAttachmentCount = 1u;
		subpasses[0].pColorAttachments = &colorReference;
		subpasses[0].pResolveAttachments = nullptr;
		subpasses[0].pDepthStencilAttachment = nullptr;
		subpasses[0].preserveAttachmentCount = 0u;
		subpasses[0].pPreserveAttachments = nullptr;

		// Subpass dependencies
		std::vector<VkSubpassDependency> dependencies(2);
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the 2d render pass
		VkRenderPassCreateInfo renderPassCInfo;
		renderPassCInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCInfo.pNext = nullptr;
		renderPassCInfo.flags = 0;
		renderPassCInfo.attachmentCount = uint32_t(attachments.size());
		renderPassCInfo.pAttachments = attachments.data();
		renderPassCInfo.subpassCount = uint32_t(subpasses.size());
		renderPassCInfo.pSubpasses = subpasses.data();
		renderPassCInfo.dependencyCount = uint32_t(dependencies.size());
		renderPassCInfo.pDependencies = dependencies.data();

		VK_CHECK(vkCreateRenderPass(LDEVICE, &renderPassCInfo, ALLOCATOR, &renderPass2D));

		// Free memory
		attachments.~vector();
		subpasses.~vector();
		dependencies.~vector();
	}

	// Create render buffers
	{
		for (size_t i = 0u; i < buffers2D.size(); i++) {
			// Create the command buffer
			buffers2D[i].execBuffer.Create(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			// Now set tjhe actual attachment resources for the framebuffer to use
			std::vector<VkImageView> attachments = {
				pSwapchain->buffers[i].imageView
			};

			// Create the frame buffer of this swapchain buffer
			VkFramebufferCreateInfo framebufferCInfo;
			framebufferCInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCInfo.pNext = nullptr;
			framebufferCInfo.flags = 0;
			framebufferCInfo.renderPass = renderPass2D;
			framebufferCInfo.attachmentCount = uint32_t(attachments.size());
			framebufferCInfo.pAttachments = attachments.data();
			framebufferCInfo.width = pSwapchain->settings.extent.width;
			framebufferCInfo.height = pSwapchain->settings.extent.height;
			framebufferCInfo.layers = 1u;

			VK_CHECK(vkCreateFramebuffer(LDEVICE, &framebufferCInfo, ALLOCATOR, &(buffers2D[i].framebuffer)));
		}
	}

	// Create 2d render specific semaphore
	VkSemaphoreCreateInfo semCInfo = initializers::semaphoreCreateInfo();
	if (semaphores.imageRendered2D == VK_NULL_HANDLE) {
		VK_CHECK(vkCreateSemaphore(LDEVICE, &semCInfo, ALLOCATOR, &semaphores.imageRendered2D));
	}

	// Create semaphore that is needed for 2d and 3d if it wasnt already created with Create2D method
	if (semaphores.imageAvailable == VK_NULL_HANDLE) {
		VK_CHECK(vkCreateSemaphore(LDEVICE, &semCInfo, ALLOCATOR, &semaphores.imageAvailable));
	}

	isCreated2D = true;
}

void vulkan::Renderer::Resize(std::vector<Object*> const& objectsToDraw)
{
	// Wait until device is idle
	vkDeviceWaitIdle(LDEVICE);

	// Let the depth image release its data
	pDepthImage->Release();

	// Free the render pass, command buffers and their framebuffers for 2d and 3d
	if (isCreated3D) {
		for (size_t i = 0u; i < buffers3D.size(); i++) {
			buffers3D[i].execBuffer.Release();
			vkDestroyFramebuffer(LDEVICE, buffers3D[i].framebuffer, ALLOCATOR);
		}
		vkDestroyRenderPass(LDEVICE, renderPass3D, ALLOCATOR);
	}
	if (isCreated2D) {
		for (size_t i = 0u; i < buffers2D.size(); i++) {
			buffers2D[i].execBuffer.Release();
			vkDestroyFramebuffer(LDEVICE, buffers2D[i].framebuffer, ALLOCATOR);
		}
		vkDestroyRenderPass(LDEVICE, renderPass2D, ALLOCATOR);
	}

	// [TURNING_POINT] Let the swapchain recreate itself 
	pSwapchain->Create();

	// Recreate the renderer that were created before
	bool tempIsCreated2D = isCreated2D;
	bool tempIsCreated3D = isCreated3D;
	isCreated2D = false;
	isCreated3D = false;

	if (tempIsCreated3D) {
		Create3D();
	}
	if (tempIsCreated2D) {
		Create2D();
	}

	// Record the objects again
	RecordDrawCommands(objectsToDraw);
}

void vulkan::Renderer::CreateShaderModule(char const* fileName, VkShaderModule& shaderModuleOut) const
{
	// Read the file code in
	std::vector<char> code = EE::tools::readFile(fileName);

	VkShaderModuleCreateInfo cinfo;
	cinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	cinfo.pNext = nullptr;
	cinfo.flags = 0;
	cinfo.codeSize = code.size();
	cinfo.pCode = (uint32_t const*)code.data();
	VK_CHECK(vkCreateShaderModule(LDEVICE, &cinfo, ALLOCATOR, &shaderModuleOut));
}

void vulkan::Renderer::RecordDrawCommands(std::vector<Object*> const& objects)
{
	// Clear values are the same over all buffers
	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	VkClearValue depthClearValue = { 1.0f, 0 }; // Depth, stencil
	VkClearValue clearValues[] = { clearColor, depthClearValue };
	VkRect2D scissor;
	scissor.offset = { 0,0 };
	scissor.extent = { pSwapchain->settings.extent.width,pSwapchain->settings.extent.height };

	// Are set to the same size in the constructor
	assert(buffers3D.size() == buffers2D.size());

	// Recording
	for (size_t i = 0u; i < buffers3D.size(); i++) {
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = pSwapchain->settings.extent;

		if (isCreated3D) {
			renderPassBeginInfo.renderPass = renderPass3D;
			renderPassBeginInfo.framebuffer = buffers3D[i].framebuffer;
			renderPassBeginInfo.clearValueCount = 2u;
			renderPassBeginInfo.pClearValues = clearValues;
			buffers3D[i].execBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
			vkCmdBeginRenderPass(buffers3D[i].execBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
		if (isCreated2D) {
			renderPassBeginInfo.renderPass = renderPass2D;
			renderPassBeginInfo.framebuffer = buffers2D[i].framebuffer;
			renderPassBeginInfo.clearValueCount = 1u;
			renderPassBeginInfo.pClearValues = &clearColor;
			buffers2D[i].execBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
			vkCmdBeginRenderPass(buffers2D[i].execBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Set scissor
		if (isCreated2D)vkCmdSetScissor(buffers2D[i].execBuffer, 0u, 1u, &scissor);
		if (isCreated3D)vkCmdSetScissor(buffers3D[i].execBuffer, 0u, 1u, &scissor);

		// Default viewport
		VkViewport vp;
		vp.x = 0.0f;
		vp.y = 0.0f;
		vp.width = float(pSwapchain->settings.extent.width);
		vp.height = float(pSwapchain->settings.extent.height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		// Change viewports size according to splitscreen mode
		if (settings.splitscreen & EE_SPLITSCREEN_MODE_VERTICAL) {
			vp.width /= 2.0f;
		}
		if (settings.splitscreen & EE_SPLITSCREEN_MODE_HORIZONTAL) {
			vp.height /= 2.0f;
		}

		// Record each object with its viewports position defined by the splitscreen position of this object
		if (settings.splitscreen != EE_SPLITSCREEN_MODE_NONE) {
			// Iterate through all objects
			for (size_t j = 0u; j < objects.size(); j++) {
				// Compute viewport position for the current object
				vp.x = (objects[j]->splitscreen & EE_SPLITSCREEN_RIGHT)
					? float(pSwapchain->settings.extent.width / 2.0f)
					: 0.0f;
				vp.y = (objects[j]->splitscreen & EE_SPLITSCREEN_BOTTOM)
					? float(pSwapchain->settings.extent.height / 2.0f)
					: 0.0f;

				// 3D object
				if(!objects[j]->is2DObject) {
					// 3D renderer requested but not created
					if (!isCreated3D) {
						EE_PRINT("[RENDERER] 3D object requested to be rendered, but no 3D renderer created!\n");
						EE::tools::warning("[RENDERER] 3D object requested to be rendered, but no 3D renderer created!\n");
					}
					// Set the viewport and let the object record itself
					vkCmdSetViewport(buffers3D[i].execBuffer, 0u, 1u, &vp);
					objects[j]->Record(buffers3D[i].execBuffer);

				// 2D object
				} else if (isCreated2D) {
					// Set the viewport and let the object record itself
					vkCmdSetViewport(buffers2D[i].execBuffer, 0u, 1u, &vp);
					objects[j]->Record(buffers2D[i].execBuffer);

				// 2D renderer requested but not created
				} else {
					EE_PRINT("[RENDERER] 2D object requested to be rendered, but no 2D renderer created!\n");
					EE::tools::warning("[RENDERER] 2D object requested to be rendered, but no 2d renderer created!\n");
				}
			}

		// No splitscreen just one viewport valid for all objects
		} else {
			if (isCreated3D) vkCmdSetViewport(buffers3D[i].execBuffer, 0u, 1u, &vp);
			if (isCreated2D) vkCmdSetViewport(buffers2D[i].execBuffer, 0u, 1u, &vp);

			// Iterate through all objects
			for (size_t j = 0u; j < objects.size(); j++) {
				// 3D object
				if (!objects[j]->is2DObject) {
					// 3D renderer requested but not created
					if (!isCreated3D) {
						EE_PRINT("[RENDERER] 3D object requested to be rendered, but no 3D renderer created!\n");
						EE::tools::warning("[RENDERER] 3D object requested to be rendered, but no 3D renderer created!\n");
					}
					// Set the viewport and let the object record itself
					vkCmdSetViewport(buffers3D[i].execBuffer, 0u, 1u, &vp);
					objects[j]->Record(buffers3D[i].execBuffer);

				// 2D object
				} else if (isCreated2D) {
					// Set the viewport and let the object record itself
					vkCmdSetViewport(buffers2D[i].execBuffer, 0u, 1u, &vp);
					objects[j]->Record(buffers2D[i].execBuffer);

				// 2D renderer requested but not created
				} else {
					EE_PRINT("[RENDERER] 2D object requested to be rendered, but no 2D renderer created!\n");
					EE::tools::warning("[RENDERER] 2D object requested to be rendered, but no 2d renderer created!\n");
				}
			}
		}

		// End recording of render pass and the command buffer
		if (isCreated3D) {
			vkCmdEndRenderPass(buffers3D[i].execBuffer);
			buffers3D[i].execBuffer.EndRecording();
		}
		if (isCreated2D) {
			vkCmdEndRenderPass(buffers2D[i].execBuffer);
			buffers2D[i].execBuffer.EndRecording();
		}
	}
}

void vulkan::Renderer::Draw() const
{
	uint32_t imageIndex;
	pSwapchain->AcquireNextImage(semaphores.imageAvailable, &imageIndex);

	/// ODERS
	// Both enabled:
	//	 -> sem::imageAvailable  -> cmd::render3D -> sem::imageRendered3D -> cmd::render2D
	//	 -> sem::imageRenderer2D -> cmd::present2D
	// 3D only:
	//	 -> sem::imageAvailable  -> cmd::render3D -> sem::imageRendered3D -> cmd::present
	// 2D only:
	//	 -> sem::imageAvailable  -> cmd::render2D -> sem::imageRendered2D -> cmd::present
	// @where:
	// sem -> cmd: means that cmd will wait till sem was signaled
	// cmd -> sem: the sem will be signaled when cmd has finished

	// General submit info set up that is valid for 2d and 3d
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStageMask;

	VkSemaphore waitSemaphore = semaphores.imageAvailable;

	if (isCreated3D) {
		submitInfo.waitSemaphoreCount = 1u;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.commandBufferCount = 1u;
		submitInfo.pCommandBuffers = &(buffers3D[imageIndex].execBuffer.cmdBuffer);
		submitInfo.signalSemaphoreCount = 1u;
		submitInfo.pSignalSemaphores = &semaphores.imageRendered3D;
		buffers3D[imageIndex].execBuffer.Execute(&submitInfo, false);

		// Change wait semaphore to one that will be signaled when 3d process has finished
		waitSemaphore = semaphores.imageRendered3D;
	}

	if (isCreated2D) {
		submitInfo.waitSemaphoreCount = 1u;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.commandBufferCount = 1u;
		submitInfo.pCommandBuffers = &(buffers2D[imageIndex].execBuffer.cmdBuffer);
		submitInfo.signalSemaphoreCount = 1u;
		submitInfo.pSignalSemaphores = &semaphores.imageRendered2D;
		buffers2D[imageIndex].execBuffer.Execute(&submitInfo, false);

		// Change wait semaphore to one that will be signaled when 2d process has finished
		waitSemaphore = semaphores.imageRendered2D;
	}

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1u;
	presentInfo.pWaitSemaphores = &waitSemaphore;
	presentInfo.swapchainCount = 1u;
	presentInfo.pSwapchains = &(pSwapchain->swapchain);
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	VK_CHECK(vkQueuePresentKHR(EEDEVICE->AcquireQueue(PRESENT_FAMILY), &presentInfo));
}

void vulkan::Renderer::WaitTillIdle() const
{
	if (isCreated2D) {
		for (size_t i = 0u; i < buffers2D.size(); i++) {
			buffers2D[i].execBuffer.Wait();
		}
	}
	if (isCreated3D) {
		for (size_t i = 0u; i < buffers3D.size(); i++) {
			buffers3D[i].execBuffer.Wait();
		}
	}
}