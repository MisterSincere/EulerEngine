/////////////////////////////////////////////////////////////////////
// Filename: vulkanObject.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanRenderer.h"
#include "eedefs.h" //< keycodes.h

namespace EE
{
	///////////////////////////
	// FOREWARD DECLARATIONS //
	///////////////////////////
	struct Mesh;
	struct Shader;
	struct Texture;
	struct Buffer;


	struct Object
	{
		vulkan::Renderer const* pRenderer;

		Shader* pShader;
		Mesh* pMesh;

		/* @brief Descriptor set of this object */
		VkDescriptorSet descriptorSet;

		bool is2DObject;
		EESplitscreen splitscreen;

		bool isVisible{ true };

		/* @brief Indicates wether this object is created to a state where it can be used */
		bool isCreated{ false };

		/**
		 * Default constructor (no creation, only storing)
		 * 
		 * @param pRenderer			Pointer to the renderer this object will be created for
		 * @param pShader				Pointer to the shader this object will use
		 * @param pMesh					Pointer to the mesh this object will use
		 * @param splitscreen		Splitscreen field this object will be rendered on
		 **/
		Object(
			vulkan::Renderer const* pRenderer,
			Shader*									pShader,
			Mesh*										pMesh,
			EESplitscreen						splitscreen);

		/**
		 * Destructor
		 **/
		~Object();

		/**
		 * Creates this object using shader/mesh passed in to the constructor
		 * 
		 * @param bindings		List of the resources that are each associated with a specific binding
		 * @param textures		List of all current textures
		 * @param buffers			List of all current buffers
		 *
		 * @return When false check console output
		 **/
		bool Create(
			std::vector<EEObjectResourceBinding> const& bindings,
			std::vector<Texture*> const&								textures,
			std::vector<Buffer*> const&								buffers);

		/**
		 * Record this object into the passed in command buffer
		 *
		 * @param cmdBuffer		The command buffer this objects calls will be recorded to
		 **/
		void Record(VkCommandBuffer cmdBuffer);


		// Delete copy/move constructor/assignements
		Object(Object const&) = delete;
		Object(Object&&) = delete;
		Object& operator=(Object const&) = delete;
		Object& operator=(Object&&) = delete;
	};
}