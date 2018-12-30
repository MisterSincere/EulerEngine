/////////////////////////////////////////////////////////////////////
// Filename: vulkanPipeline.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanRenderer.h"

namespace EE
{
	struct Pipeline
	{
		vulkan::Renderer const* pRenderer;

		/* @brief Information about the vertex stage */
		VkPipelineShaderStageCreateInfo vertexShaderStageCInfo;
		/* @brief Information about the fragment stage */
		VkPipelineShaderStageCreateInfo fragmentShaderStageCInfo;
		/* @brief The vertex input description */
		VkPipelineVertexInputStateCreateInfo vertexInputCInfo;
		/* @brief Input assembly state obtained by the renderer (currently only Trianglelist) */
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCInfo;
		/* @brief Viewport definition obtained from the renderer */
		VkPipelineViewportStateCreateInfo viewportCInfo;
		/* @brief Rasterizer info (clockwise or counterclockwise settings) */
		VkPipelineRasterizationStateCreateInfo rasterizerCInfo;
		/* @brief Multisample information obtained from the renderer */
		VkPipelineMultisampleStateCreateInfo multisampleCInfo;
		/* @brief Color blend state obtained from the renderer (default alpha blending) */
		VkPipelineColorBlendStateCreateInfo colorBlendCInfo;
		/* @brief Dynamic states of the pipeline (viewport) */
		VkPipelineDynamicStateCreateInfo dynamicsCInfo;
		/* @brief List of the push constants (optional) */
		std::vector<VkPushConstantRange> pushConstants;

		/* @brief Depth stencil description defined by the used depth image */
		VkPipelineDepthStencilStateCreateInfo depthStencilCInfo{
			vulkan::DepthImage::depthStencilStateCInfo
		};
		/* @brief Information about the vertex bindings (predefined set in constructor) */
		VkVertexInputBindingDescription predefinedVertexBindingDesc;
		/* @brief List of the vertex attribute's descriptions (predefined set in constructor) */
		std::vector<VkVertexInputAttributeDescription> predefinedVertexAttrDescs;

		/* @brief Is true if the above infos are all initialized */
		bool isInitialized{ false };
		/* @brief Is true if the pipeline/pipelinelayout were created */
		bool isCreated{ false };

		/* @brief Vulkan handle of the pipeline layout */
		VkPipelineLayout pipelineLayout;
		/* @brief Vulkan handle of the pipeline itself */
		VkPipeline pipeline;
		/* @brief Optional cache of this pipeline */
		VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

		/**
		 * Default constructor
		 *
		 * @param pRenderer			Pointer to the renderer this pipeline is for
		 * @param vertShader		Shader module of the vertex shader
		 * @param fragShader		Shader module of the fragment shader
		 **/
		Pipeline(
			vulkan::Renderer const* pRenderer,
			VkShaderModule					vertShader,
			VkShaderModule					fragShader);

		/**
		 * Destructor
		 **/
		~Pipeline();

		/**
		 * Creates the layout and the graphics pipeline according to the initialized infos
		 * and the descriptor set layout passed in.
		 *
		 * @param pDescriptorSetLayout	Layout of the descriptor sets desired for the shader
		 * @param use2DRenderPass				Indicate wether which renderer should be used
		 **/
		void Create(VkDescriptorSetLayout* pDescriptorSetLayout, bool use2DRenderPass);

		/**
		 * Call before the create method so it can use the here created pipeline cache
		 **/
		void CreatePipelineCache();


		// Delete copy constructor / assignements
		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;
	};
}