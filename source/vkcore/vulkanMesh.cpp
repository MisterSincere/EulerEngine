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

	VkDeviceSize newVertexBufferSize = static_cast<VkDeviceSize>(bufferSize);
	VkDeviceSize newIndexBufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * indices.size());

	// Create the staging buffer to upload the new vertex data
	VkBuffer stagingBufferVertices;
	VkDeviceMemory stagingBufferVerticesMemory;
	VK_CHECK(EEDEVICE->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
																	newVertexBufferSize, &stagingBufferVertices, &stagingBufferVerticesMemory, pData));

	// Also create a staging buffer for the new index data
	VkBuffer stagingBufferIndices;
	VkDeviceMemory stagingBufferIndicesMemory;
	VK_CHECK(EEDEVICE->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
																	newIndexBufferSize, &stagingBufferIndices, &stagingBufferIndicesMemory, indices.data()));

	// Transfer the data
	vulkan::ExecBuffer execBuffer(EEDEVICE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, true);
	
	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0u;
	copyRegion.dstOffset = 0u;
	// Copy vertex data
	copyRegion.size = newVertexBufferSize;
	vkCmdCopyBuffer(execBuffer.cmdBuffer, stagingBufferVertices, vertexBuffer.buffer, 1u, &copyRegion);
	// Copy index data
	copyRegion.size = newIndexBufferSize;
	vkCmdCopyBuffer(execBuffer.cmdBuffer, stagingBufferIndices, indexBuffer.buffer, 1u, &copyRegion);

	execBuffer.EndRecording();
	execBuffer.Execute();

	// Free staging buffer
	vkFreeMemory(LDEVICE, stagingBufferIndicesMemory, ALLOCATOR);
	vkDestroyBuffer(LDEVICE, stagingBufferIndices, ALLOCATOR);
	vkFreeMemory(LDEVICE, stagingBufferVerticesMemory, ALLOCATOR);
	vkDestroyBuffer(LDEVICE, stagingBufferVertices, ALLOCATOR);
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