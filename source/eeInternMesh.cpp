/////////////////////////////////////////////////////////////////////
// Filename: eeInternMesh.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


using namespace vk;



InternMesh::InternMesh(vk::VulkanRenderer* renderer)
  : renderer(renderer)
{
  assert(renderer);
}

InternMesh::~InternMesh()
{
  if (isCreated)
  {
    vkFreeMemory(renderer->swapchain->device->logicalDevice, indexBuffer.memory, renderer->swapchain->device->pAllocator);
    vkDestroyBuffer(renderer->swapchain->device->logicalDevice, indexBuffer.buffer, renderer->swapchain->device->pAllocator);
    vkFreeMemory(renderer->swapchain->device->logicalDevice, vertexBuffer.memory, renderer->swapchain->device->pAllocator);
    vkDestroyBuffer(renderer->swapchain->device->logicalDevice, vertexBuffer.buffer, renderer->swapchain->device->pAllocator);

    isCreated = false;
  }
}

void InternMesh::Create(const char* file)
{
  if (isCreated)
  {
    EEPRINT("Mesh already created. Mesh creation aborted!\n");
    return;
  }

  // Will store the vertices
  std::vector<EEInternVertex> vertices;
  std::vector<uint32_t> indices;

  tinyobj::attrib_t vertexAttributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string errorString;

  if (!tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &errorString, file)) {
    throw std::runtime_error(errorString);
  }

  std::unordered_map<EEInternVertex, size_t> vertexMap;

  for (tinyobj::shape_t shape : shapes)
  {
    for (tinyobj::index_t index : shape.mesh.indices)
    {
      glm::vec3 pos = {
        vertexAttributes.vertices[3 * index.vertex_index + 0],
        vertexAttributes.vertices[3 * index.vertex_index + 2],
        vertexAttributes.vertices[3 * index.vertex_index + 1]
      };

      glm::vec3 normal = {
        vertexAttributes.normals[3 * index.normal_index + 0],
        vertexAttributes.normals[3 * index.normal_index + 2],
        vertexAttributes.normals[3 * index.normal_index + 1]
      };


      EEInternVertex vert(pos, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, normal);

      if (vertexMap.count(vert) == 0)
      {
        vertexMap[vert] = vertexMap.size();
        vertices.push_back(vert);
      }
      indices.push_back(static_cast<uint32_t>(vertexMap[vert]));
    }
  }

  // Store amount of indices
  indexBuffer.count = static_cast<uint32_t>(indices.size());

  // Create vertex buffer
  VkDeviceSize bufferSize = sizeof(EEInternVertex) * vertices.size();
  renderer->swapchain->device->CreateAndUploadBuffer(vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    &(vertexBuffer.buffer), &(vertexBuffer.memory));

  // Create index buffer
  bufferSize = sizeof(uint32_t) * indexBuffer.count;
  renderer->swapchain->device->CreateAndUploadBuffer(indices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    &(indexBuffer.buffer), &(indexBuffer.memory));

  // Indicate that this mesh was successfully created
  isCreated = true;
}

void InternMesh::Create(const std::vector<EEVertex>& vertices, const std::vector<uint32_t>& indices)
{
  if (isCreated)
  {
    EEPRINT("Mesh already created. Mesh creation aborted!\n");
    return;
  }

  // Will store the vertices
  std::vector<EEInternVertex> _vertices;
  std::vector<uint32_t> _indices;

  // Copy vertex and index data into our vectors
  for (auto& vertex : vertices)
  {
    _vertices.push_back({ vertex.position, vertex.color, vertex.uvCoord, vertex.normal });
  }
  for (auto& index : indices)
  {
    _indices.push_back(index);
  }

  // Store amount of indices
  indexBuffer.count = static_cast<uint32_t>(_indices.size());

  // Create vertex buffer
  vertexBuffer.maxBufferSize = static_cast<VkDeviceSize>(sizeof(EEInternVertex) * _vertices.size());
  renderer->swapchain->device->CreateAndUploadBuffer(_vertices.data(), vertexBuffer.maxBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    &(vertexBuffer.buffer), &(vertexBuffer.memory));

  // Create index buffer
  indexBuffer.maxBufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * indexBuffer.count);
  renderer->swapchain->device->CreateAndUploadBuffer(_indices.data(), indexBuffer.maxBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    &(indexBuffer.buffer), &(indexBuffer.memory));

  // Indicate that this mesh was successfully created
  isCreated = true;
}

void InternMesh::Create(void* data, size_t bufferSize, std::vector<uint32_t>& indices)
{
  if (isCreated)
  {
    EEPRINT("Mesh already created. Mesh creation aborted!\n");
    return;
  }

  // Store amount of indices
  indexBuffer.count = static_cast<uint32_t>(indices.size());

  // Create vertex buffer
  vertexBuffer.maxBufferSize = static_cast<VkDeviceSize>(bufferSize);
  renderer->swapchain->device->CreateAndUploadBuffer(data, vertexBuffer.maxBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    &(vertexBuffer.buffer), &(vertexBuffer.memory));

  // Create index buffer
  indexBuffer.maxBufferSize = static_cast<VkDeviceSize>(sizeof(uint32_t) * indexBuffer.count);
  renderer->swapchain->device->CreateAndUploadBuffer(indices.data(), indexBuffer.maxBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    &(indexBuffer.buffer), &(indexBuffer.memory));

  // Indicate that this mesh was successfully created
  isCreated = true;
}
    
void InternMesh::Record(VkCommandBuffer cmdBuffer)
{
  // Bind buffer
  VkDeviceSize offset{ 0 };
  vkCmdBindVertexBuffers(cmdBuffer, 0u, 1u, &vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

  // Draw indexed
  vkCmdDrawIndexed(cmdBuffer, indexBuffer.count, 1u, 0u, 0u, 0u);
}