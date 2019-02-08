/////////////////////////////////////////////////////////////////////
// Filename: vulkanMesh.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanRenderer.h"

namespace EE
{
	struct Mesh
	{
		/* @brief The renderer this mesh uses */
		vulkan::Renderer const* pRenderer;

		/* @brief Holds informations about the vertex buffer */
		struct VertexBuffer{
			VkDeviceSize bufferSize{ 0u };
			VkBuffer buffer;
			VkDeviceMemory memory;
		};

		/* @brief Holds informations about the index buffer */
		struct IndexBuffer {
			uint32_t count;
			VkDeviceSize bufferSize{ 0u };
			VkBuffer buffer;
			VkDeviceMemory memory;
		};

		std::vector<VertexBuffer> vertexBuffers;
		uint32_t curVertexBuffer{ 0u };
		bool changeVertexBuffer{ false };
		std::vector<IndexBuffer> indexBuffers;
		uint32_t curIndexBuffer{ 0u };
		bool changeIndexBuffer{ false };


		/* @brief Indicates wether this mesh can be used */
		bool isCreated{ false };

		/**
		 * Default constructor
		 *
		 * @param pRenderer		Pointer to the renderer this mesh uses
		 **/
		Mesh(vulkan::Renderer const* pRenderer);

		/**
		 * Destructor
		 **/
		~Mesh();

		/**
		 * Creates the mesh from an obj wavefront file
		 *
		 * @TODO:: Not implemented yet, cause no priority here
		 **/
		void Create(char const* fileName);

		/**
		 * Creates a mesh from the void data and indices passed in.
		 * The data passed in needs to match the custom defined vertex input descriptions
		 * the shader was created with.
		 *
		 * @param pData				Pointer to the vertex data
		 * @param bufferSize	Size of the vertex data in bytes
		 * @param indices			List of indices (always have to be uint32_t)
		 **/
		void Create(
			void const*									 pData,
			size_t											 bufferSize,
			std::vector<uint32_t> const& indices);

		/**
		 * Updates the data of the mesh.
		 *
		 * @TODO:: Not yet working correctly (i.e. new sizes not stored or just not checked)
		 *
		 * @param pData				Pointer to the new vertex data
		 * @param bufferSize	Size of the new vertex data
		 * @param indices			List of the new indices
		 **/
		void Update(
			void const*									 pData,
			size_t											 bufferSize,
			std::vector<uint32_t> const& indices);

		/**
		 * Records the draw calls of this mesh using the previous recorded shader
		 * on the passed in command buffer.
		 *
		 * @param cmdBuffer		Command buffer this mesh will be record on
		 **/
		void Record(VkCommandBuffer cmdBuffer);


		/* @brief Delete copy/move constructor/assignements */
		Mesh(Mesh const&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh const&) = delete;
		Mesh& operator=(Mesh&&) = delete;
	};
}