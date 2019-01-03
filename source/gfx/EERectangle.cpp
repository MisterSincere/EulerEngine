/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EERectangle.h"

#include <string>

using namespace GFX;

EERectangle::EERectangle(EEApplication* pApp, EEPoint32F const& pos /*= { 0.0f, 0.0f }*/, EERect32U const& size /*= { 0.0f, 0.0f }*/)
	: i_pApp(pApp)
	, i_position(pos)
	, i_size(size)
{
	if (!pApp) {
		EE_PRINT("[EERectangle] Passed in application is invalid!\n");
		return;
	}

	EEShaderInputDesc inputDesc;
	inputDesc.location = 0;
	inputDesc.format = EE_FORMAT_R32G32_SFLOAT;
	inputDesc.offset = offsetof(Vertex, pos);

	EEVertexInput shaderInput;
	shaderInput.amountInputs = 1u;
	shaderInput.pInputDescs = &inputDesc;
	shaderInput.inputStride = sizeof(EERectangle::Vertex);

	EEDescriptorDesc descriptor;
	descriptor.type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor.shaderStage = EE_SHADER_STAGE_FRAGMENT;
	descriptor.binding = 0u;

	EEShaderCreateInfo shaderCInfo;
	shaderCInfo.vertexFileName = EE_ASSETS_DIR("shader/colorShader.vert");
	shaderCInfo.fragmentFileName = EE_ASSETS_DIR("shader/colorShader.frag");
	shaderCInfo.amountObjects = 1u;
	shaderCInfo.shaderInputType = EE_SHADER_INPUT_TYPE_CUSTOM;
	shaderCInfo.pVertexInput = &shaderInput;
	shaderCInfo.amountDescriptors = 1u;
	shaderCInfo.pDescriptors = &descriptor;
	shaderCInfo.pPushConstant = nullptr;
	shaderCInfo.is2DShader = EE_TRUE;
	shaderCInfo.wireframe = EE_FALSE;
	shaderCInfo.clockwise = EE_TRUE;
}

EERectangle::~EERectangle()
{
}