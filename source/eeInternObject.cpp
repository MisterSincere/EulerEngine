/////////////////////////////////////////////////////////////////////
// Filename: eeInternObject.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"



namespace vk
{
  namespace intern
  {

    Object::Object(vk::VulkanRenderer* renderer, vk::intern::Mesh* mesh, vk::intern::Shader* shader, EESplitscreen splitscreen)
      : renderer(renderer), mesh(mesh), shader(shader), splitscreen(splitscreen)
    {
      assert(renderer && mesh && shader);
    }

    Object::~Object()
    {
      if (isCreated)
      {
        for (size_t i = 0; i < descriptorSetDetails.uniformBuffers.size(); i++)
        {
          vkFreeMemory(renderer->swapchain->device->logicalDevice, descriptorSetDetails.uniformBuffers[i]->memory, renderer->swapchain->device->pAllocator);
          vkDestroyBuffer(renderer->swapchain->device->logicalDevice, descriptorSetDetails.uniformBuffers[i]->buffer, renderer->swapchain->device->pAllocator);

          // Free memory
          delete descriptorSetDetails.uniformBuffers[i];
        }

        // Release vector
        descriptorSetDetails.uniformBuffers.~vector();

        isCreated = false;
      }
    }

    void Object::Create(const std::vector<vk::intern::Texture*>& textures)
    {
      if (isCreated)
      {
        EEPRINT("Object was already created. Object creation aborted!\n");
        return;
      }

      // Obtain our very own desriptor set for this object :)
      shader->CreateDescriptorSet(descriptorSetDetails, textures);

      // Thats all so we are created
      isCreated = true;
    }

    void Object::Record(VkCommandBuffer cmdBuffer)
    {
      // Bind shader
      shader->Record(cmdBuffer, descriptorSetDetails.descriptorSet);

      // Bind mesh (does draw call)
      mesh->Record(cmdBuffer);
    }

    void Object::UpdateUniformBuffer(void* data, int binding)
    {
      for (size_t i = 0; i < descriptorSetDetails.uniformBuffers.size(); i++)
      {
        if (descriptorSetDetails.uniformBuffers[i]->binding == binding)
        {
          void* pData;
          vkMapMemory(renderer->swapchain->device->logicalDevice, descriptorSetDetails.uniformBuffers[i]->memory, 0, descriptorSetDetails.uniformBuffers[i]->size, 0, &pData);
          memcpy(pData, data, descriptorSetDetails.uniformBuffers[i]->size);
          vkUnmapMemory(renderer->swapchain->device->logicalDevice, descriptorSetDetails.uniformBuffers[i]->memory);
        }
      }
      EEPRINT("EulerEngine Error: No uniform buffer found at binding %d!\n", binding);
    }

  }
}