/////////////////////////////////////////////////////////////////////
// Filename: vulkanShader.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanShader.h"

#include <map>

#include "vulkanResources.h"

/* @brief Defines for better code readibility */
#define EEDEVICE (pRenderer->pSwapchain->pDevice)
#define LDEVICE (*EEDEVICE)
#define ALLOCATOR (EEDEVICE->pAllocator)


EE::Shader::Shader(vulkan::Renderer const* pRenderer, EEShaderCreateInfo const& shaderCInfo)
	: pRenderer(pRenderer)
{
	if (!pRenderer) {
		EE_PRINT("[SHADER] No valid renderer!\n");
		assert(pRenderer);
	}

	// Store settings
	settings.is2DShader = shaderCInfo.is2DShader;
	settings.maxObjects = shaderCInfo.amountObjects;
	settings.amountDescriptors = shaderCInfo.amountDescriptors;
	settings.pDescriptors = new EEDescriptorDesc[settings.amountDescriptors];
	for (uint32_t i = 0u; i < settings.amountDescriptors; i++) {
		settings.pDescriptors[i].type = shaderCInfo.pDescriptors[i].type;
		settings.pDescriptors[i].shaderStage = shaderCInfo.pDescriptors[i].shaderStage;
		settings.pDescriptors[i].binding = shaderCInfo.pDescriptors[i].binding;
	}

	// Store push constant data or not :-)
	if (shaderCInfo.pPushConstant) {
		pushConstant.pData = shaderCInfo.pPushConstant->pData;
		pushConstant.size = shaderCInfo.pPushConstant->size;
		pushConstant.shaderStage = shaderCInfo.pPushConstant->shaderStage;
	} else {
		pushConstant.pData = nullptr;
		pushConstant.size = 0u;
		pushConstant.shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	// Create the desired shader modules
	pRenderer->CreateShaderModule(shaderCInfo.vertexFileName, vertexShaderModule);
	pRenderer->CreateShaderModule(shaderCInfo.fragmentFileName, fragmentShaderModule);

	// Initialize the pipeline
	{
		pPipeline = new Pipeline(pRenderer, vertexShaderModule, fragmentShaderModule);

		// Shader input
		if (shaderCInfo.shaderInputType == EE_SHADER_INPUT_TYPE_CUSTOM) {
			// Input binding description
			VkVertexInputBindingDescription* bindingDescription = new VkVertexInputBindingDescription;
			bindingDescription->binding = 0;
			bindingDescription->stride = shaderCInfo.pShaderInput->inputStride;
			bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			// Input attribute descriptions
			VkVertexInputAttributeDescription* aAttributeDescriptions =
				new VkVertexInputAttributeDescription[shaderCInfo.pShaderInput->amountInputs];
			for (uint32_t i = 0u; i < shaderCInfo.pShaderInput->amountInputs; i++) {
				aAttributeDescriptions[i].binding = 0;
				aAttributeDescriptions[i].location = shaderCInfo.pShaderInput->pShaderInputs[i].location;
				aAttributeDescriptions[i].offset = shaderCInfo.pShaderInput->pShaderInputs[i].offset;
				aAttributeDescriptions[i].format = tools::eeToVk(shaderCInfo.pShaderInput->pShaderInputs[i].format);
			}

			pPipeline->vertexInputCInfo.pVertexBindingDescriptions = bindingDescription;
			pPipeline->vertexInputCInfo.vertexAttributeDescriptionCount = shaderCInfo.pShaderInput->amountInputs;
			pPipeline->vertexInputCInfo.pVertexAttributeDescriptions = aAttributeDescriptions;
		}

		// Push constants
		if (shaderCInfo.pPushConstant) {
			pPipeline->pushConstants.resize(1);
			pPipeline->pushConstants[0].stageFlags = tools::eeToVk(shaderCInfo.pPushConstant->shaderStage);
			pPipeline->pushConstants[0].offset = 0u;
			pPipeline->pushConstants[0].size = shaderCInfo.pPushConstant->size;
		}

		// Enable wireframe if desired
		if (shaderCInfo.wireframe) {
			if (EEDEVICE->enabledFeatures.fillModeNonSolid) {
				pPipeline->rasterizerCInfo.polygonMode = VK_POLYGON_MODE_LINE;
			} else {
				EE_PRINT("[SHADER] Wireframe rendering desired but this feature was not enabled or is not available!\n");
			}
		}

		// Change front face to be counter-clockwise if desired
		if (!shaderCInfo.clockwise) {
			pPipeline->rasterizerCInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}
	}
}

EE::Shader::~Shader()
{
	if (isCreated) {
		delete pPipeline;

		// Free the descriptor pool and descriptor set layout if descriptors were used on this shader
		if (settings.amountDescriptors) {
			vkDestroyDescriptorPool(LDEVICE, descriptorPool, ALLOCATOR);
			vkDestroyDescriptorSetLayout(LDEVICE, descriptorSetLayout, ALLOCATOR);
		}

		// Free the shader modules
		vkDestroyShaderModule(LDEVICE, vertexShaderModule, ALLOCATOR);
		vkDestroyShaderModule(LDEVICE, fragmentShaderModule, ALLOCATOR);

		// Free other memory from heap
		delete[] settings.pDescriptors; //< Were allocated in the descriptor to store the settings
		delete[] aImageInfos;
		delete[] aBufferInfos;
		delete[] aWriteDescriptorSets;

		isCreated = false;
	}
}

bool EE::Shader::Create()
{
	if (isCreated) {
		EE_PRINT("[SHADER] Was already created!\n");
		return true;
	}

	// Compatible renderer (2d or 3d) needs to be available
	if (settings.is2DShader && !pRenderer->isCreated2D) {
		EE_PRINT("[SHADER] 2D object shader requested but no 2D renderer was created!\n");
		tools::warning("[SHADER] 2D object shader requested but no 2D renderer was created!\n");
	} else if (!settings.is2DShader && !pRenderer->isCreated3D) {
		EE_PRINT("[SHADER] 3D object shader requested but no 3D renderer was created!\n");
		tools::warning("[SHADER] 3D object shader requested but no 3D renderer was created!\n");
	}

	// Create the descriptor set layout, if descriptors are used
	if (settings.amountDescriptors) {
		// Will hold the informations about the desired bindings
		VkDescriptorSetLayoutBinding* aDescriptorSetLayoutBindings =
			new VkDescriptorSetLayoutBinding[settings.amountDescriptors];

		// Will hold the desired type/shader stage for each iteration/descriptor
		VkDescriptorType type;
		VkShaderStageFlags shaderStage;
		for (uint32_t i = 0u; i < settings.amountDescriptors; i++) {
			aDescriptorSetLayoutBindings[i] = vulkan::initializers::descriptorSetLayoutBinding(
				/*type*/				tools::eeToVk(settings.pDescriptors[i].type),
				/*shaderStage*/	tools::eeToVk(settings.pDescriptors[i].shaderStage),
				/*binding*/			settings.pDescriptors[i].binding
			);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCInfo;
		descriptorSetLayoutCInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCInfo.pNext = nullptr;
		descriptorSetLayoutCInfo.flags = 0;
		descriptorSetLayoutCInfo.bindingCount = settings.amountDescriptors;
		descriptorSetLayoutCInfo.pBindings = aDescriptorSetLayoutBindings;
		VK_CHECK(vkCreateDescriptorSetLayout(LDEVICE, &descriptorSetLayoutCInfo,
																				 ALLOCATOR, &descriptorSetLayout));

		// Free the memory from the bindings array
		delete[] aDescriptorSetLayoutBindings;
	}

	// Create also the descriptor pool if descriptors are being used
	if (settings.amountDescriptors) {
		// Map to store how often a descriptor type is desired
		std::map<EEDescriptorType, uint32_t> amountPerType;
		for (uint32_t i = 0u; i < settings.amountDescriptors; i++) {
			amountPerType[settings.pDescriptors[i].type]++;
		}

		// When knowing how often each type is needed we can use that information 
		// to push back the pool sizes
		std::vector<VkDescriptorPoolSize> poolSizes;
		if (amountPerType[EE_DESCRIPTOR_TYPE_SAMPLER]) {
			poolSizes.push_back(vulkan::initializers::descriptorPoolSize(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, amountPerType[EE_DESCRIPTOR_TYPE_SAMPLER]
			));
		}
		if (amountPerType[EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER]) {
			poolSizes.push_back(vulkan::initializers::descriptorPoolSize(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, amountPerType[EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER]
			));
		}

		VkDescriptorPoolCreateInfo descriptorPoolCInfo;
		descriptorPoolCInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCInfo.pNext = nullptr;
		descriptorPoolCInfo.flags = 0;
		descriptorPoolCInfo.maxSets = settings.maxObjects;
		descriptorPoolCInfo.poolSizeCount = uint32_t(poolSizes.size());
		descriptorPoolCInfo.pPoolSizes = poolSizes.data();
		VK_CHECK(vkCreateDescriptorPool(LDEVICE, &descriptorPoolCInfo, ALLOCATOR, &descriptorPool));

		// Provide the info to easy allocate a descriptor set
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.pNext = nullptr;
		descriptorSetAllocInfo.descriptorPool = descriptorPool;
		descriptorSetAllocInfo.descriptorSetCount = 1u;
		descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

		// Pre allocation for the updatedescriptorset method, so it does not need to reallocate and delete
		// everytime the method is called
		aWriteDescriptorSets = new VkWriteDescriptorSet[settings.amountDescriptors];
		aImageInfos = new VkDescriptorImageInfo[amountPerType[EE_DESCRIPTOR_TYPE_SAMPLER]];
		aBufferInfos = new VkDescriptorBufferInfo[amountPerType[EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER]];
	}

	// Now create the pipeline
	pPipeline->Create((settings.amountDescriptors) ? &descriptorSetLayout : nullptr, settings.is2DShader);

	// Shader is successfully created and descriptor sets can be created and updated to the
	// data desired until the maximum of objects passed in is reached
	isCreated = true;
	return isCreated;
}

bool EE::Shader::CreateDescriptorSet(VkDescriptorSet* pDescriptorSet)
{
	// If this shader was set to use no descriptors return before we do sth bad :-(
	if (!settings.amountDescriptors) {
		EE_PRINT("[SHADER] Tried to create a descriptor set from a shader that was set to use no descriptors!\n");
		return true;
	}

	// Check if there are any descriptor sets left to be allocated
	if (curAmountDescriptorSets >= settings.maxObjects) {
		EE_PRINT("[SHADER] The maximum of objects that can use this shader was reached!\n");
		return false;
	}
	curAmountDescriptorSets++;

	VK_CHECK(vkAllocateDescriptorSets(LDEVICE, &descriptorSetAllocInfo, pDescriptorSet));

	return true;
}

bool EE::Shader::UpdateDescriptorSet(VkDescriptorSet descriptorSet,
																		 std::vector<EEObjectResourceBinding> const& bindings,
																		 std::vector<Texture*> const& textures,
																		 std::vector<Buffer*> const& buffers)
{
	// Indices: one iterating over the bufferinfos and the other over the imageinfos
	// @note All descriptors are composed of imageinfos and bufferinfos but not sorted,
	// thats why we use different iterators
	uint32_t bufferIterator{ 0u }, imageIterator{ 0u };

	// Loop over all desired bindings
	for (size_t i = 0u; i < bindings.size(); i++) {
		aWriteDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		aWriteDescriptorSets[i].pNext = nullptr;
		aWriteDescriptorSets[i].dstSet = descriptorSet;
		aWriteDescriptorSets[i].dstBinding = bindings[i].binding;
		aWriteDescriptorSets[i].dstArrayElement = 0u;
		aWriteDescriptorSets[i].descriptorCount = 1u;

		if (bindings[i].type == EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			// Create the desired buffer
			buffers[*bindings[i].resource]->Create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			// Overwrite the buffer info
			aBufferInfos[bufferIterator].buffer = buffers[*bindings[i].resource]->buffer;
			aBufferInfos[bufferIterator].offset = 0;
			aBufferInfos[bufferIterator].range = buffers[*bindings[i].resource]->bufferSize;

			// Set the current writedescriptorsetinfo to use this buffer
			aWriteDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			aWriteDescriptorSets[i].pImageInfo = nullptr;
			aWriteDescriptorSets[i].pBufferInfo = &aBufferInfos[bufferIterator++];
			aWriteDescriptorSets[i].pTexelBufferView = nullptr;

		} else if (bindings[i].type == EE_DESCRIPTOR_TYPE_SAMPLER) {
			// Overwrite the image info
			aImageInfos[imageIterator].sampler = textures[*bindings[i].resource]->sampler;
			aImageInfos[imageIterator].imageView = textures[*bindings[i].resource]->imageView;
			aImageInfos[imageIterator].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// Set the current writedescriptorsetinfo to use this image
			aWriteDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			aWriteDescriptorSets[i].pImageInfo = &aImageInfos[imageIterator++];
			aWriteDescriptorSets[i].pBufferInfo = nullptr;
			aWriteDescriptorSets[i].pTexelBufferView = nullptr;
		}
	}

	// Finally update the descriptor set to the newly created buffers and precreated images
	// specified withing the bindings list
	vkUpdateDescriptorSets(LDEVICE, settings.amountDescriptors, aWriteDescriptorSets, 0u, nullptr);

	return true;
}

void EE::Shader::Record(VkCommandBuffer cmdBuffer, VkDescriptorSet* pDescriptorSet)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->pipeline);

	if (pushConstant.pData) {
		vkCmdPushConstants(cmdBuffer, pPipeline->pipelineLayout, pushConstant.shaderStage,
											 0u, pushConstant.size, pushConstant.pData);
	}
	if (settings.amountDescriptors && pDescriptorSet) {
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->pipelineLayout,
														0u, 1u, pDescriptorSet, 0u, nullptr);
	}
}