/////////////////////////////////////////////////////////////////////
// Filename: vulkanPipeline.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanPipeline.h"

#include <cassert>

#include "vulkanResources.h"

/* @brief Defines for better code readibility */
#define EEDEVICE pRenderer->pSwapchain->pDevice
#define LDEVICE (*EEDEVICE)
#define ALLOCATOR EEDEVICE->pAllocator

EE::Pipeline::Pipeline(vulkan::Renderer const* pRenderer, VkShaderModule vertShader, VkShaderModule fragShader)
	: pRenderer(pRenderer)
{
	if (!pRenderer) {
		EE_PRINT("[PIPELINE] No valid renderer!\n");
		assert(pRenderer);
	}

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
	vertexInputCInfo.vertexAttributeDescriptionCount = uint32_t(predefinedVertexAttrDescs.size());
	vertexInputCInfo.pVertexAttributeDescriptions = predefinedVertexAttrDescs.data();

	// VkPipelineInputAssemblyStateCreateInfo
	inputAssemblyCInfo = pRenderer->inputAssemblyState;

	// VkPipelineViewportStateCreateInfo
	viewportCInfo = pRenderer->viewportState;

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
	multisampleCInfo = pRenderer->multisampleState;

	// VkPipelineColorBlendStateCreateInfo
	colorBlendCInfo = pRenderer->blendState;

	// VkPipelineDynamicStateCreateinfo
	dynamicsCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicsCInfo.pNext = nullptr;
	dynamicsCInfo.flags = 0;
	dynamicsCInfo.dynamicStateCount = 1u;
	dynamicsCInfo.pDynamicStates = new VkDynamicState{ VK_DYNAMIC_STATE_VIEWPORT };

	// Use descriptions of the predefined vertex type
	predefinedVertexBindingDesc = EE::PredefinedVertex::inputBindingDescription;
	predefinedVertexAttrDescs = EE::PredefinedVertex::inputAttributeDescriptions;

	// We initialized everything
	isInitialized = true;
}

EE::Pipeline::~Pipeline()
{
	if (dynamicsCInfo.pDynamicStates) delete dynamicsCInfo.pDynamicStates;

	if (isCreated) {
		vkDestroyPipeline(LDEVICE, pipeline, ALLOCATOR);
		vkDestroyPipelineLayout(LDEVICE, pipelineLayout, ALLOCATOR);
		if (pipelineCache != VK_NULL_HANDLE) vkDestroyPipelineCache(LDEVICE, pipelineCache, ALLOCATOR);

		// Free memory
		delete[] vertexInputCInfo.pVertexBindingDescriptions;
		delete[] vertexInputCInfo.pVertexAttributeDescriptions;

		isCreated = false;
	}
}

void EE::Pipeline::Create(VkDescriptorSetLayout* pDescriptorSetLayout, bool use2D)
{
	if (!isInitialized) {
		EE_PRINT("[PIPELINE] Creation failed pipeline was not initialzed!\n");
		tools::exitFatal("[PIPELINE] Creation failed pipeline was not initialzed!\n");
	}
	if (!isCreated) {
		EE_PRINT("[PIPELINE] Already created!\n");
		tools::warning("[PIPELINE] Already created!\n");
		return;
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		vertexShaderStageCInfo, fragmentShaderStageCInfo
	};

	// Create the pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCInfo;
	pipelineLayoutCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCInfo.pNext = nullptr;
	pipelineLayoutCInfo.flags = 0;
	pipelineLayoutCInfo.setLayoutCount = 1u;
	pipelineLayoutCInfo.pSetLayouts = pDescriptorSetLayout;
	pipelineLayoutCInfo.pushConstantRangeCount = uint32_t(pushConstants.size());
	pipelineLayoutCInfo.pPushConstantRanges = pushConstants.data();
	VK_CHECK(vkCreatePipelineLayout(LDEVICE, &pipelineLayoutCInfo, ALLOCATOR, &pipelineLayout));

	// Determine renderpass that should be used
	VkRenderPass usedRenderPass = (use2D) ? pRenderer->renderPass2D : pRenderer->renderPass3D;
	if (use2D && !pRenderer->isCreated2D) {
		EE_PRINT("[PIPELINE] 2D renderer desired but was never created!\n");
		tools::exitFatal("[PIPELINE] 2D renderer desired but was never created!\n");
	} else if (!use2D && !pRenderer->isCreated3D) {
		EE_PRINT("[PIPELINE] 3D renderer desired but was never created!\n");
		tools::exitFatal("[PIPELINE] 3D renderer desired but was never created!\n");
	}

	// Finally create the pipeline itself
	VkGraphicsPipelineCreateInfo pipelineCInfo;
	pipelineCInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCInfo.pNext = nullptr;
	pipelineCInfo.flags = 0;
	pipelineCInfo.stageCount = uint32_t(shaderStages.size());
	pipelineCInfo.pStages = shaderStages.data();
	pipelineCInfo.pVertexInputState = &vertexInputCInfo;
	pipelineCInfo.pInputAssemblyState = &inputAssemblyCInfo;
	pipelineCInfo.pTessellationState = nullptr;
	pipelineCInfo.pViewportState = &viewportCInfo;
	pipelineCInfo.pRasterizationState = &rasterizerCInfo;
	pipelineCInfo.pMultisampleState = &multisampleCInfo;
	pipelineCInfo.pDepthStencilState = &depthStencilCInfo;
	pipelineCInfo.pColorBlendState = &colorBlendCInfo;
	pipelineCInfo.pDynamicState = &dynamicsCInfo;
	pipelineCInfo.layout = pipelineLayout;
	pipelineCInfo.renderPass = usedRenderPass;
	pipelineCInfo.subpass = 0;
	pipelineCInfo.basePipelineHandle = VK_NULL_HANDLE; // If used at some time, set flags to derivative bit
	pipelineCInfo.basePipelineIndex = -1;
	VK_CHECK(vkCreateGraphicsPipelines(LDEVICE, pipelineCache, 1u, &pipelineCInfo, ALLOCATOR, &pipeline));

	isCreated = true;
}

void EE::Pipeline::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCInfo;
	pipelineCacheCInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheCInfo.pNext = nullptr;
	pipelineCacheCInfo.flags = 0;
	pipelineCacheCInfo.initialDataSize = 0;
	pipelineCacheCInfo.pInitialData = nullptr;
	VK_CHECK(vkCreatePipelineCache(LDEVICE, &pipelineCacheCInfo, ALLOCATOR, &pipelineCache));
}