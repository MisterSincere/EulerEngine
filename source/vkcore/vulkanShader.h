/////////////////////////////////////////////////////////////////////
// Filename: vulkanShader.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanPipeline.h"


namespace EE
{
	///////////////////////////
	// FOREWARD DECLARATIONS //
	///////////////////////////
	struct Texture;
	struct Buffer;

	struct Shader
	{
		/* @brief Renderer this shader is for */
		vulkan::Renderer const* pRenderer;

		/* @brief The pipeline that was created for this shader */
		Pipeline* pPipeline;

		/* @brief Shader modules for the modifiable different shader stages */
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;
		/* @brief Desired descriptor set layout of this shader */
		VkDescriptorSetLayout descriptorSetLayout;
		/* @brief Pool providing memory for all desired descriptor sets */
		VkDescriptorPool descriptorPool;
		/* @brief Allocation description for a descriptor set */
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo;
		/* @brief Pre-Allocation of the write descriptor infos */
		VkWriteDescriptorSet* aWriteDescriptorSets;
		/* @brief Pre-Allocation of needed image infos */
		VkDescriptorImageInfo* aImageInfos;
		/* @brief Pre-Allocation of needed buffer infos */
		VkDescriptorBufferInfo* aBufferInfos;

		/* @brief Holds settings of this shader needed during the whole lifetime of this shader */
		struct {
			bool is2DShader;
			uint32_t maxObjects;
			uint32_t amountDescriptors{ 0u };
			EEDescriptorDesc* pDescriptors;
		} settings;

		/* @brief Encapsulates infos around the desired push constant */
		struct {
			void* pData;
			uint32_t size;
			VkShaderStageFlags shaderStage;
		} pushConstant;

		/* @brief Indicates wether this shader is usable */
		bool isCreated{ false };

		/* @brief Amoun of already allocated descriptorsets to check if max was reached */
		uint32_t curAmountDescriptorSets{ 0u };

		/**
		 * Default constructor
		 *
		 * @param pRenderer		Pointer of the renderer to use
		 * @param shaderCInfo	Information about the desired shader settings
		 **/
		Shader(vulkan::Renderer const* pRenderer, EEShaderCreateInfo const& shaderCInfo);

		/**
		 * Destructor
		 **/
		~Shader();

		/**
		 * Will create the descirptor setlayouts/pool etc. and the pipeline
		 *
		 * @return If a renderer (2d/3d) was requested but never created this returns false
		 **/
		bool Create();

		/**
		 * Creates a descriptor set for an object to use
		 *
		 * @param pDescriptorSetOut		Returned descriptor set allocated from this pool
		 *
		 * @return Is false if the max amount objects was exceeded
		 **/
		bool CreateDescriptorSet(VkDescriptorSet* pDescriptorSetOut);

		/**
		 * Updates the descriptor set to the desired data
		 *
		 * @param descriptorSet		The descriptor set that is to update
		 * @param bindings				Resources that will be bound to the descriptor set
		 * @param	textures				List of all current textures
		 * @param buffers					List of all current buffers
		 **/
		bool UpdateDescriptorSet(
			VkDescriptorSet															descriptorSet,
			std::vector<EEObjectResourceBinding> const& bindings,
			std::vector<Texture*> const&								textures,
			std::vector<Buffer*> const&									buffers) const;

		/**
		 * Records this shader into the passed in command buffer
		 *
		 * @param cmdBuffer			Command buffer this shader will be recorded to
		 * @param pDescriptorSet	If descriptors are used in this shader this set will be recorded too
		 **/
		void Record(VkCommandBuffer cmdBuffer, VkDescriptorSet const* pDescriptorSet = nullptr) const;


		/* @brief Delete copy/move constructor/assignements */
		Shader(Shader const&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(Shader const&) = delete;
		Shader& operator=(Shader&&) = delete;
	};
}