/////////////////////////////////////////////////////////////////////
// Filename: eeInternShader.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"

#include <map>


namespace vk
{
  namespace intern
  {
    
    Shader::Shader(vk::VulkanRenderer* renderer, const EEShaderCreateInfo& shaderCInfo)
      : renderer(renderer), shaderInfo(shaderCInfo)
    {
      assert(renderer);

      // Store push constant data
      if (shaderInfo.pPushConstantDesc)
      {
        pushConstant.data = shaderInfo.pPushConstantDesc->value;
        pushConstant.size = shaderInfo.pPushConstantDesc->size;
      }
      else
      {
        pushConstant.data = nullptr;
        pushConstant.size = 0u;
      }
    }

    Shader::~Shader()
    {
      if (isCreated)
      {
        // DESCRIPTOR POOL
        vkDestroyDescriptorPool(renderer->swapchain->device->logicalDevice, descriptorPool, renderer->swapchain->device->pAllocator);

        // PIPELINE
        delete pipeline;

        // DESCRIPTOR SET LAYOUT
        vkDestroyDescriptorSetLayout(renderer->swapchain->device->logicalDevice, descriptorSetLayout, renderer->swapchain->device->pAllocator);

        // SHADER MODULES
        vkDestroyShaderModule(renderer->swapchain->device->logicalDevice, vertexShaderModule, renderer->swapchain->device->pAllocator);
        vkDestroyShaderModule(renderer->swapchain->device->logicalDevice, fragmentShaderModule, renderer->swapchain->device->pAllocator);
        if(geometryShaderModule) vkDestroyShaderModule(renderer->swapchain->device->logicalDevice, geometryShaderModule, renderer->swapchain->device->pAllocator);

        // Free memory
        delete writeDescriptorSets;
        writeDescriptorSets = nullptr;

        // Not created anymore
        isCreated = false;
      }
    }

    void Shader::Create()
    {
      if (isCreated)
      {
        EEPRINT("Shader was already created! Creation aborted!\n");
        return;
      }

      // DESCRIPTOR SET LAYOUT
      {
        // Will hold the informations about the desired bindings
        VkDescriptorSetLayoutBinding* descriptorSetLayoutBindings = new VkDescriptorSetLayoutBinding[shaderInfo.uniformCount];

        // Will hold the desired type and shader stage for each iteration
        VkDescriptorType type;
        VkShaderStageFlags stageFlags;
        for (uint32_t i = 0u; i < shaderInfo.uniformCount; i++)
        {
          type = (shaderInfo.pUniformDescs[i].uniformType == EE_UNIFORM_TYPE_BUFFER) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
          stageFlags = (shaderInfo.pUniformDescs[i].shaderStage == EE_SHADER_STAGE_VERTEX) ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;

          descriptorSetLayoutBindings[i] = vk::initializers::descriptorSetLayoutBinding(type, stageFlags, shaderInfo.pUniformDescs[i].binding);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCInfo;
        descriptorSetLayoutCInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCInfo.pNext = nullptr;
        descriptorSetLayoutCInfo.flags = 0;
        descriptorSetLayoutCInfo.bindingCount = shaderInfo.uniformCount;
        descriptorSetLayoutCInfo.pBindings = descriptorSetLayoutBindings;

        VK_CHECK(vkCreateDescriptorSetLayout(renderer->swapchain->device->logicalDevice, &descriptorSetLayoutCInfo, renderer->swapchain->device->pAllocator, &descriptorSetLayout));

        // Free memory
        delete descriptorSetLayoutBindings;
      }

      // DESCRIPTOR POOL
      {
        // Store per descriptor type how often we need them
        std::map<EEUniformType, uint32_t> amountPerDescriptorType;
        for (uint32_t i = 0u; i < shaderInfo.uniformCount; i++)
        {
          amountPerDescriptorType[shaderInfo.pUniformDescs[i].uniformType]++;
        }

        // Push back desired pool sizes
        std::vector<VkDescriptorPoolSize> poolSizes;
        if (amountPerDescriptorType[EE_UNIFORM_TYPE_SAMPLER])
        {
          poolSizes.push_back(vk::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, amountPerDescriptorType[EE_UNIFORM_TYPE_SAMPLER]));
        }
        if (amountPerDescriptorType[EE_UNIFORM_TYPE_BUFFER])
        {
          poolSizes.push_back(vk::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, amountPerDescriptorType[EE_UNIFORM_TYPE_SAMPLER]));
        }

        VkDescriptorPoolCreateInfo descriptorPoolCInfo;
        descriptorPoolCInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCInfo.pNext = nullptr;
        descriptorPoolCInfo.flags = 0;
        descriptorPoolCInfo.maxSets = shaderInfo.amountObjects;
        descriptorPoolCInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolCInfo.pPoolSizes = poolSizes.data();

        VK_CHECK(vkCreateDescriptorPool(renderer->swapchain->device->logicalDevice, &descriptorPoolCInfo, renderer->swapchain->device->pAllocator, &descriptorPool));

        // Free memory
        poolSizes.~vector();
      }

      // Fill arbitrary descriptor set allocate info
      descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      descriptorSetAllocInfo.pNext = nullptr;
      descriptorSetAllocInfo.descriptorPool = descriptorPool;
      descriptorSetAllocInfo.descriptorSetCount = 1u;
      descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

      // PIPELINE
      {
        // Create shader module
        renderer->CreateShaderModule(shaderInfo.vertexFileName, vertexShaderModule);
        renderer->CreateShaderModule(shaderInfo.fragmentFileName, fragmentShaderModule);


        // Create the pipeline with the created descriptor set layout and shader module
        pipeline = new vk::intern::Pipeline(renderer, vertexShaderModule, fragmentShaderModule);
        pipeline->Ini(shaderInfo);
        pipeline->Create(descriptorSetLayout);
      }


      // Now the shader is created
      isCreated = true;
    }

    VkBuffer pushNewUniformBuffer(vk::VulkanDevice* device, const EEUniformDesc& uniformDesc, std::vector<Shader::UniformBufferDetails*>& uniformBuffers)
    {
      Shader::UniformBufferDetails* ubo = new Shader::UniformBufferDetails;
      ubo->size = uniformDesc.bufferSize;
      ubo->binding = uniformDesc.binding;
      device->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, ubo->size,
        &(ubo->buffer), &(ubo->memory));

      uniformBuffers.push_back(ubo);

      return ubo->buffer;
    }

    void Shader::CreateDescriptorSet(DescriptorSetDetails& descriptorSetOut, const std::vector<vk::intern::Texture>& textures)
    {
      if (currentAmountDescriptorSets == shaderInfo.amountObjects)
      {
        EEPRINT("Maximum of objects for this shader reached. Object creation aborted!\nToo create more object increase amountObjects in the EEShaderCreateInfo\n");
        return;
      }
      currentAmountDescriptorSets++;

      VK_CHECK(vkAllocateDescriptorSets(renderer->swapchain->device->logicalDevice, &descriptorSetAllocInfo, &(descriptorSetOut.descriptorSet)));

      // UPDATE WRITE INFOS DESCRIPTOR SET
      {
        // Fill the different infos
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;
        for (uint32_t i = 0u; i < shaderInfo.uniformCount; i++)
        {
          // BUFFER
          if (shaderInfo.pUniformDescs[i].uniformType == EE_UNIFORM_TYPE_BUFFER)
          {
            bufferInfos.push_back({
              /*buffer*/ pushNewUniformBuffer(renderer->swapchain->device, shaderInfo.pUniformDescs[i], descriptorSetOut.uniformBuffers),
              /*offset*/ 0,
              /*range */ shaderInfo.pUniformDescs[i].bufferSize
            });

          // SAMPLER
          } else if (shaderInfo.pUniformDescs[i].uniformType == EE_UNIFORM_TYPE_SAMPLER) {
            imageInfos.push_back({
              /*sampler    */ textures[*(shaderInfo.pUniformDescs[i].texture)].sampler,
              /*imageView  */ textures[*(shaderInfo.pUniformDescs[i].texture)].imageView,
              /*imageLayout*/ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
          }

        }

        uint32_t infoAmount{ 0u };

        if (bufferInfos.size())
        {
          writeDescriptorSets[infoAmount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          writeDescriptorSets[infoAmount].pNext = nullptr;
          writeDescriptorSets[infoAmount].dstBinding = 0u;
          writeDescriptorSets[infoAmount].dstArrayElement = 0u;
          writeDescriptorSets[infoAmount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          writeDescriptorSets[infoAmount].pImageInfo = nullptr;
          writeDescriptorSets[infoAmount].pTexelBufferView = nullptr;
          writeDescriptorSets[infoAmount].dstSet = descriptorSetOut.descriptorSet;
          writeDescriptorSets[infoAmount].descriptorCount = static_cast<uint32_t>(bufferInfos.size());
          writeDescriptorSets[infoAmount].pBufferInfo = bufferInfos.data();
        }

        if (imageInfos.size())
        {
          writeDescriptorSets[infoAmount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          writeDescriptorSets[infoAmount].pNext = nullptr;
          writeDescriptorSets[infoAmount].dstBinding = 1u;
          writeDescriptorSets[infoAmount].dstArrayElement = 0u;
          writeDescriptorSets[infoAmount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
          writeDescriptorSets[infoAmount].pBufferInfo = nullptr;
          writeDescriptorSets[infoAmount].pTexelBufferView = nullptr;
          writeDescriptorSets[infoAmount].dstSet = descriptorSetOut.descriptorSet;
          writeDescriptorSets[infoAmount].descriptorCount = static_cast<uint32_t>(imageInfos.size());
          writeDescriptorSets[infoAmount].pImageInfo = imageInfos.data();
          infoAmount++;
        }

        vkUpdateDescriptorSets(renderer->swapchain->device->logicalDevice, infoAmount, writeDescriptorSets, 0u, nullptr);
      }
    }
  }
}