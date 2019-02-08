/////////////////////////////////////////////////////////////////////
// Filename: vulkanMesh.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanMesh.h"

/* @brief Defines for better code readibility */
#define EEDEVICE (pRenderer->pSwapchain->pDevice)
#define LDEVICE (*EEDEVICE)
#define ALLOCATOR (EEDEVICE->pAllocator)

#define CUR_INDEX_BUFFER (indexBuffers[curIndexBuffer])
#define CUR_VERTEX_BUFFER (vertexBuffers[curVertexBuffer])
#define OTHER_INDEX_BUFFER (indexBuffers[(curIndexBuffer + 1) % 2])
#define OTHER_VERTEX_BUFFER (vertexBuffers[(curVertexBuffer + 1) % 2])

EE::Mesh::Mesh(vulkan::Renderer const* pRenderer)
	: pRenderer(pRenderer)
{
	if (!pRenderer) {
		EE_PRINT("[MESH] Invalid renderer passed in to the constructor!\n");
		return;
	}
}

EE::Mesh::~Mesh()
{
	if (isCreated) {
		if (CUR_INDEX_BUFFER.bufferSize) {
			vkFreeMemory(LDEVICE, CUR_INDEX_BUFFER.memory, ALLOCATOR);
			vkDestroyBuffer(LDEVICE, CUR_INDEX_BUFFER.buffer, ALLOCATOR);
		}
		if (CUR_VERTEX_BUFFER.bufferSize) {
			vkFreeMemory(LDEVICE, CUR_VERTEX_BUFFER.memory, ALLOCATOR);
			vkDestroyBuffer(LDEVICE, CUR_VERTEX_BUFFER.buffer, ALLOCATOR);
		}

		isCreated = false;
	}
}

void EE::Mesh::Create(char const* fileName)
{
	if (isCreated) {
		EE_PRINT("[MESH] Already created!\n");
		return;
	}
	// @TODO
	EE_PRINT("[MESH] Obj loading function not yet implemented!\n");
}

void EE::Mesh::Create(void const* pData, size_t bufferSize, std::vector<uint32_t> const& indices)
{
	if (isCreated) {
		EE_PRINT("[MESH] Already created!\n");
		return;
	}

	// Initialize buffer buffering vectors
	vertexBuffers.resize(2);
	indexBuffers.resize(2);

	// Store amount of indices
	CUR_INDEX_BUFFER.count = uint32_t(indices.size());

	// Create the vertex buffer
	CUR_VERTEX_BUFFER.bufferSize = static_cast<VkDeviceSize>(bufferSize);
	EEDEVICE->CreateDeviceLocalBuffer(pData, CUR_VERTEX_BUFFER.bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
																		&(CUR_VERTEX_BUFFER.buffer), &(CUR_VERTEX_BUFFER.memory));
	
	// Create the index buffer
	CUR_INDEX_BUFFER.bufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * CUR_INDEX_BUFFER.count);
	EEDEVICE->CreateDeviceLocalBuffer(indices.data(), CUR_INDEX_BUFFER.bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
																		&(CUR_INDEX_BUFFER.buffer), &(CUR_INDEX_BUFFER.memory));

	// Indicate that this mesh can now be used/recorded
	isCreated = true;
}

void EE::Mesh::Update(void const* pData, size_t bufferSize, std::vector<uint32_t> const& indices)
{
	if (!isCreated) {
		EE_PRINT("[MESH] Please create the mesh before you want to update it!\n");
		return;
	}

	VkDeviceSize newVertexBufferSize = static_cast<VkDeviceSize>(bufferSize);
	VkDeviceSize newIndexBufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * indices.size());


	// VERTEX BUFFER
	if (newVertexBufferSize > 0 && newVertexBufferSize != CUR_VERTEX_BUFFER.bufferSize) {

			// Create the other vertex buffer and indicate for the next record to use this buffer
			// note: if changeVertexBuffer is already true this method was called at least twice before
			// the draw call and we need to release/destroy/free the previous "new" vertex buffer
			if (changeVertexBuffer) {
				vkFreeMemory(LDEVICE, OTHER_VERTEX_BUFFER.memory, ALLOCATOR);
				vkDestroyBuffer(LDEVICE, OTHER_VERTEX_BUFFER.buffer, ALLOCATOR);
			} else {
				changeVertexBuffer = true;
			}

			OTHER_VERTEX_BUFFER.bufferSize = newVertexBufferSize;
			EEDEVICE->CreateDeviceLocalBuffer(pData, OTHER_VERTEX_BUFFER.bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				&(OTHER_VERTEX_BUFFER.buffer), &(OTHER_VERTEX_BUFFER.memory));

	} else if (newVertexBufferSize > 0) {
			// Create the staging buffers
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VK_CHECK(EEDEVICE->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
																			newVertexBufferSize, &stagingBuffer, &stagingBufferMemory, pData));
			// Copy vertex data
			vulkan::ExecBuffer execBuffer(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);
			VkBufferCopy copyRegion;
			copyRegion.srcOffset = 0u;
			copyRegion.dstOffset = 0u;
			copyRegion.size = newVertexBufferSize;
			vkCmdCopyBuffer(execBuffer.cmdBuffer, stagingBuffer, CUR_VERTEX_BUFFER.buffer, 1u, &copyRegion);
			execBuffer.EndRecording();
			execBuffer.Execute();

			// Free staging buffers
			vkFreeMemory(LDEVICE, stagingBufferMemory, ALLOCATOR);
			vkDestroyBuffer(LDEVICE, stagingBuffer, ALLOCATOR);

	} else {
		changeVertexBuffer = true;
	}
	


	// INDEX BUFFER
	if (newIndexBufferSize > 0 && newIndexBufferSize != CUR_INDEX_BUFFER.bufferSize) {
		// Create the other index buffer and indicate for the next record to use this buffer
		// note: if changeIndexBuffer is already true this method was called at least twice before
		// the draw call and we need to release/destroy/free the previous "new" index buffer
		if (changeIndexBuffer) {
			vkFreeMemory(LDEVICE, OTHER_INDEX_BUFFER.memory, ALLOCATOR);
			vkDestroyBuffer(LDEVICE, OTHER_INDEX_BUFFER.buffer, ALLOCATOR);
		} else {
			changeIndexBuffer = true;
		}

		OTHER_INDEX_BUFFER.bufferSize = newIndexBufferSize;
		OTHER_INDEX_BUFFER.count = uint32_t(indices.size());
		EEDEVICE->CreateDeviceLocalBuffer(indices.data(), OTHER_INDEX_BUFFER.bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			&(OTHER_INDEX_BUFFER).buffer, &(OTHER_INDEX_BUFFER).memory);

	} else if (newIndexBufferSize > 0) {
		// Also create a staging buffers
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VK_CHECK(EEDEVICE->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
																		newIndexBufferSize, &stagingBuffer, &stagingBufferMemory, indices.data()));
		// Copy index data
		vulkan::ExecBuffer execBuffer(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);
		VkBufferCopy copyRegion;
		copyRegion.srcOffset = 0u;
		copyRegion.dstOffset = 0u;
		copyRegion.size = newIndexBufferSize;
		vkCmdCopyBuffer(execBuffer.cmdBuffer, stagingBuffer, CUR_INDEX_BUFFER.buffer, 1u, &copyRegion);
		execBuffer.EndRecording();
		execBuffer.Execute();

		// Free staging buffers
		vkFreeMemory(LDEVICE, stagingBufferMemory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, stagingBuffer, ALLOCATOR);

	} else {
		changeIndexBuffer = true;
	}
}

void EE::Mesh::Record(VkCommandBuffer cmdBuffer)
{
	// Will destroy the current vertex buffer if it exists and will switch to the other one
	// if it exists. Will only be entered once, per "set-to-true" of changeVertexBuffer
	if (changeVertexBuffer) {
		if (CUR_VERTEX_BUFFER.bufferSize) {
			vkFreeMemory(LDEVICE, CUR_VERTEX_BUFFER.memory, ALLOCATOR);
			vkDestroyBuffer(LDEVICE, CUR_VERTEX_BUFFER.buffer, ALLOCATOR);
			CUR_VERTEX_BUFFER.bufferSize = 0;
		}

		if (OTHER_VERTEX_BUFFER.bufferSize) {
			curVertexBuffer = (curVertexBuffer + 1) % 2;
		}

		changeVertexBuffer = false;
	}

	// Similiar to the vertex buffer check above
	if (changeIndexBuffer) {
		if (CUR_INDEX_BUFFER.bufferSize) {
			vkFreeMemory(LDEVICE, CUR_INDEX_BUFFER.memory, ALLOCATOR);
			vkDestroyBuffer(LDEVICE, CUR_INDEX_BUFFER.buffer, ALLOCATOR);
			CUR_INDEX_BUFFER.bufferSize = 0;
		}

		// Only switch if other exists
		if (OTHER_INDEX_BUFFER.bufferSize) {
			curIndexBuffer = (curIndexBuffer + 1) % 2;
		}

		changeIndexBuffer = false;
	}

	// Draw this mesh if the index/vertex buffer exist
	if (CUR_VERTEX_BUFFER.bufferSize && CUR_INDEX_BUFFER.bufferSize) {
		// Bind buffer
		VkDeviceSize offset{ 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0u, 1u, &(CUR_VERTEX_BUFFER.buffer), &offset);
		vkCmdBindIndexBuffer(cmdBuffer, CUR_INDEX_BUFFER.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Draw indexed
		vkCmdDrawIndexed(cmdBuffer, CUR_INDEX_BUFFER.count, 1u, 0u, 0u, 0u);
	}
}