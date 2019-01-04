/////////////////////////////////////////////////////////////////////
// Filename: vulkanResources.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanResources.h"

#include <algorithm>

///////////////
// STB IMAGE //
///////////////
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* @brief Defines for better code readibility */
#define EEDEVICE (pRenderer->pSwapchain->pDevice)
#define LDEVICE (*EEDEVICE)
#define ALLOCATOR (EEDEVICE->pAllocator)


//-------------------------------------------------------------------
// Predefined Vertex
//-------------------------------------------------------------------
VkVertexInputBindingDescription EE::PredefinedVertex::inputBindingDescription = {
	/*binding*/		0,
	/*stride*/		sizeof(EEPredefinedVertex),
	/*inputRate*/ VK_VERTEX_INPUT_RATE_VERTEX
};

std::vector<VkVertexInputAttributeDescription> EE::PredefinedVertex::inputAttributeDescriptions = {
	{
		/*location*/ 0,
		/*binding*/	 0,
		/*format*/	 VK_FORMAT_R32G32B32_SFLOAT,
		/*offset*/	 offsetof(EEPredefinedVertex, position)
	},
	{
		/*location*/ 1,
		/*binding*/	 0,
		/*format*/	 VK_FORMAT_R32G32B32_SFLOAT,
		/*offset*/	 offsetof(EEPredefinedVertex, color)
	},
	{
		/*location*/ 2,
		/*binding*/	 0,
		/*format*/	 VK_FORMAT_R32G32_SFLOAT,
		/*offset*/	 offsetof(EEPredefinedVertex, uvCoord)
	},
	{
		/*location*/ 3,
		/*binding*/	 0,
		/*format*/	 VK_FORMAT_R32G32B32_SFLOAT,
		/*offset*/	 offsetof(EEPredefinedVertex, normal)
	},
};

bool EE::PredefinedVertex::operator==(EE::PredefinedVertex const& other) const
{
	bool posEqual = pos.x == other.pos.x && pos.y == other.pos.y && pos.z == other.pos.z;
	bool colEqual = col.x == other.col.x && col.y == other.col.y && col.z == other.col.z;
	bool uvEqual = uv.x == other.uv.x && uv.y == other.uv.y;
	bool normEqual = norm.x == other.norm.x && norm.y == other.norm.y && norm.z == other.norm.z;
	return posEqual && colEqual && uvEqual && normEqual;
}


//-------------------------------------------------------------------
// Texture
//-------------------------------------------------------------------
EE::Texture::Texture(vulkan::Renderer const* pRenderer, char const* fileName,
										 bool mipMapping, bool unnormalizedCoordinates)
	: pRenderer(pRenderer)
{
	if (!pRenderer) {
		EE_PRINT("[TEXTURE] No valid renderer was passed in!\n");
		assert(pRenderer);
	}

	// Store option to not use normalized coords
	this->unnormalizedCoordinates = unnormalizedCoordinates;

	// Get the primitive data of the image
	int width, height;
	data.pixels = stbi_load(fileName, &width, &height, &data.channels, STBI_rgb_alpha);
	if (!data.pixels) {
		EE_PRINT("[TEXTURE] Image not found: %s\n", fileName);
		tools::exitFatal("Failed to find image!\n");
	}
	// For some reason stbi_load sets channel to 3 although alpha is requested
	data.channels = 4;

	format = VK_FORMAT_R8G8B8A8_UNORM;
	data.width = uint32_t(width);
	data.height = uint32_t(height);

	if (mipMapping) {
		// Check if linear blitting is supported since it is needed for generating the different
		// mip levels. If not keep the miplevel value to 1
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(EEDEVICE->physicalDevice, format, &formatProperties);
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) {
			mipLevels = uint32_t(std::floor(std::log2(std::max(width, height)))) + 1u;
		}
	}

	// Data loaded and different vk settings specified
	isInitialized = true;
}

EE::Texture::Texture(vulkan::Renderer const* pRenderer, EETextureCreateInfo const& textureCInfo)
	: pRenderer(pRenderer)
{
	if (!pRenderer) {
		EE_PRINT("[TEXTURE] No valid renderer was passed in!\n");
		assert(pRenderer);
	}

	// Store option to not use normalized coords
	unnormalizedCoordinates = textureCInfo.unnormalizedCoordinates;

	// Store dimensions
	data.width = textureCInfo.extent.width;
	data.height = textureCInfo.extent.height;

	// Query not for the correct alpha channel
	switch (textureCInfo.format)
	{
	case EE_FORMAT_R8_UINT:
		format = VK_FORMAT_R8_UINT;
		data.channels = 1;
		break;

	case EE_FORMAT_R8_UNORM:
		format = VK_FORMAT_R8_UNORM;
		data.channels = 1;
		break;

	case EE_FORMAT_R8G8B8A8_UINT:
		format = VK_FORMAT_R8G8B8A8_UINT;
		data.channels = 4;
		break;

	case EE_FORMAT_R8G8B8A8_UNORM:
		format = VK_FORMAT_R8G8B8A8_UNORM;
		data.channels = 4;
		break;

	default:
		format = VK_FORMAT_UNDEFINED;
		data.channels = -1;
	}

	if (format == VK_FORMAT_UNDEFINED) {
		EE_PRINT("[TEXTURE] Unsupported format passed in!\n");
		tools::warning("[TEXTURE] Unsupported format passed in!\n");
		return;
	}

	// Allocate memory for the data to be stored
	uint64_t bufferSize = sizeof(unsigned char) * data.channels * data.width * data.height;
	data.pixels = new unsigned char[bufferSize];
	memcpy_s(data.pixels, bufferSize, textureCInfo.pData, bufferSize);

	isInitialized = true;
}

EE::Texture::~Texture()
{
	if (isInitialized) {
		delete[] data.pixels;

		isInitialized = false;
	}

	if (isUploaded) {
		vkDestroySampler(LDEVICE, sampler, ALLOCATOR);
		vkDestroyImageView(LDEVICE, imageView, ALLOCATOR);
		vkDestroyImage(LDEVICE, image, ALLOCATOR);
		vkFreeMemory(LDEVICE, imageMemory, ALLOCATOR);

		isUploaded = false;
	}
}

void EE::Texture::Upload()
{
	if (isUploaded) {
		EE_PRINT("[TEXTURE] Texture already uploaded!\n");
		return;
	}

	// Compute image size
	VkDeviceSize imageSize = data.width * data.height * data.channels;

	// Create the pure (empty) image and allocate its memory
	{
		// We will sample from this image. For creation we will also set the image to be 
		// a transfer dst, so we can transfer the image data into the sampler.
		// If we have more than one mip level we will need to generate the different mip levels
		// from the base mip level. So we would also need to set the image as a transfer src.
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (mipLevels > 1u) usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkImageCreateInfo imageCInfo;
		imageCInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCInfo.pNext = nullptr;
		imageCInfo.flags = 0;
		imageCInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCInfo.format = format;
		imageCInfo.extent = { data.width, data.height, 1u };
		imageCInfo.mipLevels = mipLevels;
		imageCInfo.arrayLayers = 1u;
		imageCInfo.samples = pRenderer->settings.sampleCount;
		imageCInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCInfo.usage = usageFlags;
		imageCInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCInfo.queueFamilyIndexCount = 0u;
		imageCInfo.pQueueFamilyIndices = nullptr;
		imageCInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VK_CHECK(vkCreateImage(LDEVICE, &imageCInfo, ALLOCATOR, &image));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(LDEVICE, image, &memReqs);
		VkMemoryAllocateInfo allocInfo = vulkan::initializers::memoryAllocateInfo();
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex =
			EEDEVICE->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vkAllocateMemory(LDEVICE, &allocInfo, ALLOCATOR, &imageMemory));

		vkBindImageMemory(LDEVICE, image, imageMemory, 0);
	}

	// Fill the image with the desired data
	{
		// Create a staging buffer with the data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		EEDEVICE->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			imageSize,
			&stagingBuffer,
			&stagingBufferMemory,
			data.pixels);

		// Get a cmd buffer that will be used to transfer the layout and copy to the image
		vulkan::ExecBuffer execBuffer(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


		// Change image's layout to transfer dst so that we can than copy to it
		vulkan::tools::imageBarrier(execBuffer.cmdBuffer,
																image,
																VK_IMAGE_ASPECT_COLOR_BIT,
																mipLevels,
																VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		// Copy from buffer to image
		vulkan::tools::bufferImageCopy(execBuffer.cmdBuffer,
																	 stagingBuffer,
																	 image,
																	 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																	 VK_IMAGE_ASPECT_COLOR_BIT,
																	 data.width, data.height);

		// End recording and execute
		execBuffer.EndRecording();
		execBuffer.Execute();

		// Free staging buffer
		vkDestroyBuffer(LDEVICE, stagingBuffer, ALLOCATOR);
		vkFreeMemory(LDEVICE, stagingBufferMemory, ALLOCATOR);
	}

	// Finish the image
	// The image will be changed to a shaderreadonlyoptimal image.
	// Also if different mip levels are desired these will be created before
	// setting the imagelayout change.
	{
		vulkan::ExecBuffer execBuffer(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// If there are multiple mipmap levels generate these
		if (mipLevels > 1u) {
			vulkan::tools::generateMipmaps(execBuffer.cmdBuffer,
																		 image,
																		 format,
																		 data.width, data.height,
																		 mipLevels);
		} else {
			vulkan::tools::imageBarrier(execBuffer.cmdBuffer,
																	image,
																	VK_IMAGE_ASPECT_COLOR_BIT,
																	1u,
																	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
																	VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
		}

		// End recording and execute
		execBuffer.EndRecording();
		execBuffer.Execute();
	}

	// Create the image view
	{
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
		imageViewCInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCInfo.subresourceRange.baseMipLevel = 0u;
		imageViewCInfo.subresourceRange.levelCount = mipLevels;
		imageViewCInfo.subresourceRange.baseArrayLayer = 0u;
		imageViewCInfo.subresourceRange.layerCount = 1u;
		VK_CHECK(vkCreateImageView(LDEVICE, &imageViewCInfo, ALLOCATOR, &imageView));
	}

	// Create the sampler for the shader
	{
		VkSamplerCreateInfo samplerCInfo;
		samplerCInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCInfo.pNext = nullptr;
		samplerCInfo.flags = 0;
		samplerCInfo.magFilter = VK_FILTER_LINEAR;
		samplerCInfo.minFilter = VK_FILTER_LINEAR;
		samplerCInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCInfo.mipLodBias = 0.0f;
		samplerCInfo.maxAnisotropy = 16.0f;
		samplerCInfo.compareEnable = VK_FALSE;
		samplerCInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCInfo.minLod = 0.0f;
		samplerCInfo.maxLod = (unnormalizedCoordinates) ? 0.0f : uint32_t(mipLevels);
		samplerCInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCInfo.unnormalizedCoordinates = unnormalizedCoordinates;

		if (unnormalizedCoordinates) {
			samplerCInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerCInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		} else {
			samplerCInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
		// Enable anistotropic filter if suported by device
		if (EEDEVICE->enabledFeatures.samplerAnisotropy) {
			samplerCInfo.anisotropyEnable = (unnormalizedCoordinates) ? VK_FALSE : VK_TRUE;
		} else {
			samplerCInfo.anisotropyEnable = VK_FALSE;
		}

		VK_CHECK(vkCreateSampler(LDEVICE, &samplerCInfo, ALLOCATOR, &sampler));
	}

	isUploaded = true;
}


//-------------------------------------------------------------------
// Buffer
//-------------------------------------------------------------------
EE::Buffer::Buffer(vulkan::Device const* pDevice, size_t bufferSize)
	: pDevice(pDevice)
	, bufferSize(bufferSize)
{
	if (!pDevice) {
		EE_PRINT("[BUFFER] No valid device passed in!\n");
		assert(pDevice);
	}
}

EE::Buffer::~Buffer()
{
	if (isCreated) {
		// Free the vulkan buffer handles
		vkFreeMemory(pDevice->logicalDevice, bufferMemory, pDevice->pAllocator);
		vkDestroyBuffer(pDevice->logicalDevice, buffer, pDevice->pAllocator);

		isCreated = false;
	}
}

void EE::Buffer::Create(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties)
{
	// Store settings
	settings.usage = usage;
	settings.memoryProperties = memoryProperties;

	// Create the buffer
	VK_CHECK(pDevice->CreateBuffer(settings.usage,
		settings.memoryProperties,
		bufferSize,
		&buffer, &bufferMemory));

	isCreated = true;
}

void EE::Buffer::Update(void const* pData) const
{
	if (!isCreated) {
		EE_PRINT("[BUFFER] Buffer wasn't created!\n");
		return;
	}

	void* mapped;
	VK_CHECK(vkMapMemory(pDevice->logicalDevice, bufferMemory, 0, bufferSize, 0, &mapped));
	memcpy(mapped, pData, bufferSize);
	vkUnmapMemory(pDevice->logicalDevice, bufferMemory);
}
