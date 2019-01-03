/////////////////////////////////////////////////////////////////////
// Filename: vulkanTools.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanTools.h"

#include <Windows.h>
#include <cassert>
#include <fstream>

#include "Graphics.h"
#include "eedefs.h"

using namespace EE;


std::vector<char> tools::readFile(char const* fileName)
{
	std::ifstream file;
	file.open(fileName, std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		size_t size = static_cast<size_t>(file.tellg());
		std::vector<char> fileBuffer(size);
		file.seekg(0);
		file.read(fileBuffer.data(), size);
		file.close();
		return fileBuffer;
	}
	else
	{
		EE_PRINT("Failed to open file %s!\n", fileName);
		exitFatal("Failed to open shader file!\n");
		return std::vector<char>();
	}
}

void tools::exitFatal(char const* msg)
{
#ifdef UNICODE
#define EE_STR_TEMP(val) L##val
#else
#define EE_STR_TEMP(val) val
#endif
	MessageBox(nullptr, EE_STR_TEMP(msg), EE_STR_TEMP("Fatal Error"), MB_OK | MB_ICONERROR);
	assert(false);
}

void tools::warning(char const* msg)
{
	MessageBox(nullptr, EE_STR_TEMP(msg), EE_STR_TEMP("WARNING"), MB_OK | MB_ICONERROR);
#undef EE_STR_TEMP
}

VkFormat tools::eeToVk(EEFormat format)
{
#define FCASE(name) case EE_FORMAT_##name: return VK_FORMAT_##name;
	switch (format)
	{
		FCASE(R32G32_SFLOAT);
		FCASE(R32G32B32_SFLOAT);
		FCASE(R8_UINT);
		FCASE(R8_UNORM);
		FCASE(R8G8B8A8_UINT);
		FCASE(R8G8B8A8_UNORM);
	default: return VK_FORMAT_UNDEFINED;
	}
#undef FCASE
}

VkShaderStageFlags tools::eeToVk(EEShaderStage shaderStage)
{
#define SCASE(name) case EE_SHADER_STAGE_##name: return VK_SHADER_STAGE_##name##_BIT;
	switch (shaderStage)
	{
		SCASE(VERTEX);
		SCASE(FRAGMENT);
	default: return VK_SHADER_STAGE_ALL_GRAPHICS;
	}
#undef SCASE
}

VkDescriptorType tools::eeToVk(EEDescriptorType type)
{
	switch (type)
	{
	case EE_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}

bool vulkan::tools::isStencilFormat(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT;
}

char const* vulkan::tools::vkResultToStr(VkResult res)
{
	switch (res) {
#define STR(val) case val: return #val;
		STR(VK_ERROR_OUT_OF_HOST_MEMORY);
		STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
		STR(VK_ERROR_INITIALIZATION_FAILED);
		STR(VK_ERROR_LAYER_NOT_PRESENT);
		STR(VK_ERROR_EXTENSION_NOT_PRESENT);
		STR(VK_ERROR_FEATURE_NOT_PRESENT);
		STR(VK_ERROR_TOO_MANY_OBJECTS);
		STR(VK_ERROR_DEVICE_LOST);
		STR(VK_ERROR_INCOMPATIBLE_DRIVER);
		STR(VK_ERROR_SURFACE_LOST_KHR);
		STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
#undef STR
	default:
		return "UNKNOWN ERROR";
	}
}

void vulkan::tools::imageBarrier(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspectMask, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = 0u;
	imageMemoryBarrier.dstQueueFamilyIndex = 0u;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
	imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
	imageMemoryBarrier.subresourceRange.layerCount = 1u;

	// Actions that need to be finished before it will be
	// transitioned to the new layout
	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined or does not matter (only valid if initial layout)
		imageMemoryBarrier.srcAccessMask = 0u;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is ready by a shader
		// Make sure any shade reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	default:
		EE_PRINT("Image layout transition not supported: Invalid old layout!\n");
		EE::tools::exitFatal("Image layout transition not supported: Invalid old layout!");
		break;
	}

	// Target layouts
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finsihed
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image will be used as a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		break;

	default:
		EE_PRINT("Image layout transition not yet supported: Invalid new layout!\n");
		EE::tools::exitFatal("Image layout transition not yet supported: Invalid new layout!");
		break;
	}

	vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);
}

void vulkan::tools::bufferImageCopy(VkCommandBuffer cmdBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout,
	VkImageAspectFlags aspectMask, uint32_t width, uint32_t height)
{
	VkBufferImageCopy copyRegion;
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0u;
	copyRegion.bufferImageHeight = 0u;
	copyRegion.imageSubresource.aspectMask = aspectMask;
	copyRegion.imageSubresource.mipLevel = 0u;
	copyRegion.imageSubresource.baseArrayLayer = 0u;
	copyRegion.imageSubresource.layerCount = 1u;
	copyRegion.imageOffset = { 0, 0, 0 };
	copyRegion.imageExtent = { width, height, 1u };
	vkCmdCopyBufferToImage(cmdBuffer, srcBuffer, dstImage, dstImageLayout, 1u, &copyRegion);
}

void vulkan::tools::generateMipmaps(VkCommandBuffer cmdBuffer, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = static_cast<int32_t>(width);
	int32_t mipHeight = static_cast<int32_t>(height);

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(cmdBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
}