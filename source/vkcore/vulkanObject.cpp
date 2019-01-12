/////////////////////////////////////////////////////////////////////
// Filename: vulkanObject.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanObject.h"

#include <cassert>

#include "vulkanShader.h"
#include "vulkanMesh.h"

/* @brief Defines for better code readibility */
#define EEDEVICE pRenderer->pSwapchain->pDevice
#define LDEVICE *(EEDEVICE)
#define ALLOCATOR EEDEVICE->pAllocator


EE::Object::Object(vulkan::Renderer const* pRenderer, Shader* pShader, Mesh* pMesh, EESplitscreen splitscreen)
	: pRenderer(pRenderer)
	, pMesh(pMesh)
	, pShader(pShader)
	, splitscreen(splitscreen)
{
	assert(pRenderer && pShader && pMesh);
	is2DObject = pShader->settings.is2DShader;
}

EE::Object::~Object()
{
	if (isCreated) {
		// Free only our own descriptor sets since the shader/mesh can be used by other objects
		vkFreeDescriptorSets(LDEVICE, pShader->descriptorPool, 1u, &descriptorSet);
		// Decrease the current amount of object using the use shader
		pShader->curAmountDescriptorSets--;

		isCreated = false;
	}
}

bool EE::Object::Create(std::vector<EEObjectResourceBinding> const& bindings,
												std::vector<Texture*> const& textures,
												std::vector<Buffer*> const& buffers)
{
	if (isCreated) {
		EE_PRINT("[OBJECT] Was already created. Aborted creation call!\n");
		return false;
	}

	// Obtain a descriptor set for this object, if there are any defined for the current shader
	if (pShader->settings.amountDescriptors) {
		if (!pShader->CreateDescriptorSet(&descriptorSet)) return false;
		if (!pShader->UpdateDescriptorSet(descriptorSet, bindings, textures, buffers)) return false;
	}

	// Thats all so this object is created
	isCreated = true;

	return true;
}

void EE::Object::Record(VkCommandBuffer cmdBuffer)
{
	if (!isVisible) return;

	// Record shader
	pShader->Record(cmdBuffer, (pShader->settings.amountDescriptors) ? &descriptorSet : nullptr);

	// Record now the mesh and its draw call
	pMesh->Record(cmdBuffer);
}