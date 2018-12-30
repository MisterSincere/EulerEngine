/////////////////////////////////////////////////////////////////////
// Filename: vulkanResources.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <DirectXMath.h>

#include "vulkanRenderer.h"

namespace EE
{
	struct PredefinedVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 col;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 norm;

		PredefinedVertex(DirectX::XMFLOAT3 const& pos,
										 DirectX::XMFLOAT3 const& color,
										 DirectX::XMFLOAT2 const& uvCoord,
										 DirectX::XMFLOAT3 const& normal)
			: pos(pos)
			, col(color)
			, uv(uvCoord)
			, norm(normal)
		{}

		bool operator==(PredefinedVertex const&) const;

		static VkVertexInputBindingDescription inputBindingDescription;
		static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;
	};

	struct Texture
	{
		/* @brief The renderer this image will be used on */
		vulkan::Renderer const* pRenderer;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;
		VkFormat format;

		/* @brief Holds the primitive data of the image */
		struct {
			unsigned char* pixels;
			uint32_t width;
			uint32_t height;
			int channels;
		} data;

		/* @brief Amount of mipmap levels */
		uint32_t mipLevels{ 1u };
		/* @brief Indicates wether this textures coordinates will be interpreted as normalized */
		bool unnormalizedCoordinates{ false };
		/* @brief Indicates that this texture is ready to be created */
		bool isInitialized{ false };
		/* @brief Indicates that this texture can be used for a shader */
		bool isUploaded{ false };

		/**
		 * Default constructor: loads the image data
		 *
		 * @param pRenderer								Pointer to the renderer to use
		 * @param fileName								Destination of the image file to load/represent
		 * @param mipMapping							If set to true mipmap levels will be created
		 * @param unnormalizedCoordinates Set to true to not use normalized coords [0,1]
		 **/
		Texture(
			vulkan::Renderer const* pRenderer,
			char const*							fileName,
			bool										mipMapping,
			bool										unnormalizedCoordinates);

		/**
		 * Constructor that initializes with information about the data passed in and the sampler
		 * (create with upload function)
		 *
		 * @param pRenderer			Pointer to the renderer to use
		 * @param textureCInfo	Information about the texture data and how to sample from it
		 **/
		Texture(vulkan::Renderer const* pRenderer, EETextureCreateInfo const& textureCInfo);

		/**
		 * Destructor
		 **/
		~Texture();

		/**
		 * Creates the vulkan handles needed to use this texture for a shader
		 **/
		void Upload();


		/* @brief Delete copy/move constructor/assignements */
		Texture(Texture const&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(Texture const&) = delete;
		Texture& operator=(Texture&&) = delete;
	};

	struct Buffer
	{
		/* @brief The device this buffer uses */
		vulkan::Device const* pDevice;

		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
		VkDeviceSize bufferSize;

		/* @brief Holds settings that are set on buffer creation */
		struct {
			VkBufferUsageFlags usage{ 0 };
			VkMemoryPropertyFlags memoryProperties{ 0 };
		} settings;

		/* @brief Indicates wether this buffer can be used */
		bool isCreated{ false };

		/**
		 * Default constructor
		 *
		 * @param pDevice			Pointer to the device this buffer uses
		 * @param bufferSize	Desired size of this buffer
		 **/
		Buffer(vulkan::Device const* pDevice, size_t bufferSize);

		/**
		 * Destructor
		 **/
		~Buffer();

		/**
		 * Creates the buffer with the set device
		 *
		 * @param usage							Desired usage of this buffer
		 * @param memoryProperties	Memory properties this buffer should have
		 **/
		void Create(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);

		/**
		 * Updates the data of the buffer
		 *
		 * @param pData		Pointer to the new data
		 **/
		void Update(void* pData);


		/* @brief Delete copy/move constructor/assignements */
		Buffer(Buffer const&) = delete;
		Buffer(Buffer&&) = delete;
		Buffer& operator=(Buffer const&) = delete;
		Buffer& operator=(Buffer&&) = delete;
	};
}