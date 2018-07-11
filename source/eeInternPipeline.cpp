/////////////////////////////////////////////////////////////////////
// Filename: eeInternPipeline.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"

using namespace vk;



InternPipeline::InternPipeline(vk::VulkanRenderer* renderer, VkShaderModule vertShader, VkShaderModule fragShader)
  : renderer(renderer)
{
  assert(renderer);

  // VERTEX: VkPipelineShaderStageCreateInfo
  vertexShaderStageCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexShaderStageCInfo.pNext = nullptr;
  vertexShaderStageCInfo.flags = 0;
  vertexShaderStageCInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertexShaderStageCInfo.module = vertShader;
  vertexShaderStageCInfo.pName = "main";
  vertexShaderStageCInfo.pSpecializationInfo = nullptr;

  // FRAGMENT: VkPipelineShaderStageCreateInfo
  vertexShaderStageCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexShaderStageCInfo.pNext = nullptr;
  vertexShaderStageCInfo.flags = 0;
  vertexShaderStageCInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  vertexShaderStageCInfo.module = fragShader;
  vertexShaderStageCInfo.pName = "main";
  vertexShaderStageCInfo.pSpecializationInfo = nullptr;

  // VkPipelineVertexInputStateCreateInfo
  vertexInputCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputCInfo.pNext = nullptr;
  vertexInputCInfo.flags = 0;
  vertexInputCInfo.vertexBindingDescriptionCount = 1u;
  vertexInputCInfo.pVertexBindingDescriptions = &predefinedVertexBindingDesc;
  vertexInputCInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(predefinedVertexAttrDescs.size());
  vertexInputCInfo.pVertexAttributeDescriptions = predefinedVertexAttrDescs.data();

  // VkPipelineInputAssemblyStateCreateInfo
  inputAssemblyCInfo = renderer->GetInputAssemblyStateCreateInfo();

  // VkPipelineViewportStateCreateInfo
  viewportCInfo = renderer->GetViewportStateCreateInfo();
      
  // VkPipelineRasterizationStateCreateInfo
  rasterizerCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizerCInfo.pNext = nullptr;
  rasterizerCInfo.flags = 0;
  rasterizerCInfo.depthClampEnable = VK_FALSE;
  rasterizerCInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizerCInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizerCInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizerCInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizerCInfo.depthBiasEnable = VK_FALSE;
  rasterizerCInfo.depthBiasConstantFactor = 0.0f;
  rasterizerCInfo.depthBiasClamp = 0.0f;
  rasterizerCInfo.depthBiasSlopeFactor = 0.0f;
  rasterizerCInfo.lineWidth = 1.0f;

  // VkPipelineMultisampleStateCreateInfo
  multisampleCInfo = renderer->GetMultisampleStateCreateInfo();

  // VkPipelineColorBlendStateCreateInfo
  blendStateCInfo = renderer->GetColorBlendStateCreateInfo();

  // VkPipelineDynamicStateCreateInfo
  std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  dynamicStateCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCInfo.pNext = nullptr;
  dynamicStateCInfo.flags = 0;
  dynamicStateCInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicStateCInfo.pDynamicStates = dynamicStates.data();

  // We initialized everything so indicate this
  isInitialized = true;
}

InternPipeline::~InternPipeline()
{
  if (isCreated)
  {
    vkDestroyPipeline(renderer->swapchain->device->logicalDevice, pipeline, renderer->swapchain->device->pAllocator);
    vkDestroyPipelineLayout(renderer->swapchain->device->logicalDevice, pipelineLayout, renderer->swapchain->device->pAllocator);

    // Free memory
    // @TODO:: Check if necessary
    delete vertexInputCInfo.pVertexBindingDescriptions;
    delete vertexInputCInfo.pVertexAttributeDescriptions;

    isCreated = false;
  }
}

void InternPipeline::Ini(const EEShaderCreateInfo& shaderCInfo)
{
  // SHADER INPUT
  if (shaderCInfo.shaderInputType == EE_SHADER_INPUT_TYPE_CUSTOM)
  {
    // Binding description
    VkVertexInputBindingDescription* bindingDescription = new VkVertexInputBindingDescription;
    bindingDescription->binding = 0;
    bindingDescription->stride = shaderCInfo.pShaderInput->inputStride;
    bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Input attribute descriptions
    VkVertexInputAttributeDescription* attributeDescriptions = new VkVertexInputAttributeDescription[shaderCInfo.pShaderInput->shaderInputDescCount];
    for (uint32_t i = 0u; i < shaderCInfo.pShaderInput->shaderInputDescCount; i++)
    {
      attributeDescriptions[i].location = shaderCInfo.pShaderInput->pShaderInputDescs[i].location;
      attributeDescriptions[i].binding = 0;
      attributeDescriptions[i].format = (shaderCInfo.pShaderInput->pShaderInputDescs[i].format == EE_FORMAT_R32G32_SFLOAT) ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[i].offset = shaderCInfo.pShaderInput->pShaderInputDescs[i].offset;
    }

    vertexInputCInfo.pVertexBindingDescriptions = bindingDescription;
    vertexInputCInfo.vertexAttributeDescriptionCount = shaderCInfo.pShaderInput->shaderInputDescCount;
    vertexInputCInfo.pVertexAttributeDescriptions = attributeDescriptions;
  }

  // PUSH CONSTANTS
  if (shaderCInfo.pPushConstantDesc)
  {
    pushConstants.resize(1);
    pushConstants[0].stageFlags = (shaderCInfo.pPushConstantDesc[0].shaderStage == EE_SHADER_STAGE_FRAGMENT) ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT;
    pushConstants[0].offset = 0u;
    pushConstants[0].size = shaderCInfo.pPushConstantDesc[0].size;
  }

  // WIREFRAME
  if (shaderCInfo.wireframe)
  {
    rasterizerCInfo.polygonMode = VK_POLYGON_MODE_LINE;
  }

  // CLOCKWISE
  if (!shaderCInfo.clockwise)
  {
    rasterizerCInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  }
}

void InternPipeline::Create(VkDescriptorSetLayout descriptorSetLayout)
{
  if (!isInitialized)
  {
    EEPRINT("Pipeline infos were not initialized! Creation aborted\n");
    return;
  }
  if (isCreated)
  {
    EEPRINT("Pipeline was already created! Creation aborted\n");
    return;
  }

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCInfo, fragmentShaderStageCInfo };

  // Pipeline Layout
  VkPipelineLayoutCreateInfo pipelineLayoutCInfo;
  pipelineLayoutCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCInfo.pNext = nullptr;
  pipelineLayoutCInfo.flags = 0;
  pipelineLayoutCInfo.setLayoutCount = 1u;
  pipelineLayoutCInfo.pSetLayouts = &descriptorSetLayout;
  pipelineLayoutCInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
  pipelineLayoutCInfo.pPushConstantRanges = pushConstants.data();

  VK_CHECK(vkCreatePipelineLayout(renderer->swapchain->device->logicalDevice, &pipelineLayoutCInfo, renderer->swapchain->device->pAllocator, &pipelineLayout));

  // Finally the pipeline
  VkGraphicsPipelineCreateInfo pipelineCInfo;
  pipelineCInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCInfo.pNext = nullptr;
  pipelineCInfo.flags = 0;
  pipelineCInfo.stageCount = 2u;
  pipelineCInfo.pStages = shaderStages;
  pipelineCInfo.pVertexInputState = &vertexInputCInfo;
  pipelineCInfo.pInputAssemblyState = &inputAssemblyCInfo;
  pipelineCInfo.pTessellationState = nullptr;
  pipelineCInfo.pViewportState = &viewportCInfo;
  pipelineCInfo.pRasterizationState = &rasterizerCInfo;
  pipelineCInfo.pMultisampleState = &multisampleCInfo;
  pipelineCInfo.pDepthStencilState = &depthStencilCInfo;
  pipelineCInfo.pColorBlendState = &blendStateCInfo;
  pipelineCInfo.pDynamicState = &dynamicStateCInfo;
  pipelineCInfo.layout = pipelineLayout;
  pipelineCInfo.renderPass = renderer->renderPass;
  pipelineCInfo.subpass = 0;
  pipelineCInfo.basePipelineHandle = VK_NULL_HANDLE;    //< If used at later point set to derivative bit
  pipelineCInfo.basePipelineIndex = -1;

  VK_CHECK(vkCreateGraphicsPipelines(renderer->swapchain->device->logicalDevice, VK_NULL_HANDLE, 1u, &pipelineCInfo, renderer->swapchain->device->pAllocator, &pipeline));

  // Now we created the pipeline
  isCreated = true;
}
