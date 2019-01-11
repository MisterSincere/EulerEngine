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
		vkFreeMemory(LDEVICE, indexBuffer.memory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, indexBuffer.buffer, ALLOCATOR);
		vkFreeMemory(LDEVICE, vertexBuffer.memory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, vertexBuffer.buffer, ALLOCATOR);

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

	// Store amount of indices
	indexBuffer.count = uint32_t(indices.size());

	// Create the vertex buffer
	vertexBuffer.bufferSize = static_cast<VkDeviceSize>(bufferSize);
	EEDEVICE->CreateDeviceLocalBuffer(pData, vertexBuffer.bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						&(vertexBuffer.buffer), &(vertexBuffer.memory));

	// Create the index buffer
	indexBuffer.bufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * indexBuffer.count);
	EEDEVICE->CreateDeviceLocalBuffer(indices.data(), indexBuffer.bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
						&(indexBuffer.buffer), &(indexBuffer.memory));

	// Indicate that this mesh can now be used/recorded
	isCreated = true;
}

void EE::Mesh::Update(void const* pData, size_t bufferSize, std::vector<uint32_t> const& indices)
{
	if (!isCreated) {
		EE_PRINT("[MESH] Please create the mesh before you want to update it!\n");
		return;
	}
	pRenderer->WaitTillIdle();

	VkDeviceSize newVertexBufferSize = static_cast<VkDeviceSize>(bufferSize);
	VkDeviceSize newIndexBufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * indices.size());

	bool allowRecreation = false;

	// VERTEX BUFFER
	if (allowRecreation && newVertexBufferSize != vertexBuffer.bufferSize) {
		vkFreeMemory(LDEVICE, vertexBuffer.memory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, vertexBuffer.buffer, ALLOCATOR);

	} else {
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
		vkCmdCopyBuffer(execBuffer.cmdBuffer, stagingBuffer, vertexBuffer.buffer, 1u, &copyRegion);
		execBuffer.EndRecording();
		execBuffer.Execute();

		// Free staging buffers
		vkFreeMemory(LDEVICE, stagingBufferMemory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, stagingBuffer, ALLOCATOR);
	}


	// INDEX BUFFER
	if (allowRecreation && newIndexBufferSize != indexBuffer.bufferSize) {
		vkFreeMemory(LDEVICE, indexBuffer.memory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, indexBuffer.buffer, ALLOCATOR);


	} else {
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
		vkCmdCopyBuffer(execBuffer.cmdBuffer, stagingBuffer, indexBuffer.buffer, 1u, &copyRegion);
		execBuffer.EndRecording();
		execBuffer.Execute();

		// Free staging buffers
		vkFreeMemory(LDEVICE, stagingBufferMemory, ALLOCATOR);
		vkDestroyBuffer(LDEVICE, stagingBuffer, ALLOCATOR);
	}
}

void EE::Mesh::Record(VkCommandBuffer cmdBuffer) const
{
	// Bind buffer
	VkDeviceSize offset{ 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0u, 1u, &vertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	// Draw indexed
	vkCmdDrawIndexed(cmdBuffer, indexBuffer.count, 1u, 0u, 0u, 0u);
}