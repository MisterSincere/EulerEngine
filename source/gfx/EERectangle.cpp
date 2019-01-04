/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EERectangle.h"

#include <EEApplication.h>
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

	std::string vert = EE_ASSETS_DIR("shader/color2DVert.spv");
	std::string frag = EE_ASSETS_DIR("shader/color2DFrag.spv");
	EEShaderCreateInfo shaderCInfo;
	shaderCInfo.vertexFileName = vert.c_str();
	shaderCInfo.fragmentFileName = frag.c_str();
	shaderCInfo.amountObjects = 1u;
	shaderCInfo.shaderInputType = EE_SHADER_INPUT_TYPE_CUSTOM;
	shaderCInfo.pVertexInput = &shaderInput;
	shaderCInfo.amountDescriptors = 1u;
	shaderCInfo.pDescriptors = &descriptor;
	shaderCInfo.pPushConstant = nullptr;
	shaderCInfo.is2DShader = EE_TRUE;
	shaderCInfo.wireframe = EE_FALSE;
	shaderCInfo.clockwise = EE_TRUE;
	m_shader = pApp->CreateShader(shaderCInfo);


	std::vector<Vertex> vertices = {
		{DirectX::XMFLOAT2(-0.5f, -0.5f)}, //< LEFT TOP
		{DirectX::XMFLOAT2(+0.5f, -0.5f)}, //< RIGHT TOP
		{DirectX::XMFLOAT2(-0.5f, +0.5f)}, //< LEFT BOTTOM
		{DirectX::XMFLOAT2(+0.5f, +0.5f)}, //< RIGHT BOTTOM
	};

	std::vector<uint32_t> indices = {
		0u, 1u, 2u,
		2u, 1u, 3u
	};
	m_mesh = pApp->CreateMesh(vertices.data(), vertices.size(), indices);


	m_uniformBuffer = pApp->CreateBuffer(sizeof(FragmentUBO));


	std::vector<EEObjectResourceBinding> bindings(1);
	bindings[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].binding = 0u;
	bindings[0].resource = m_uniformBuffer;
	m_object = pApp->CreateObject(m_shader, m_mesh, bindings);
}

EERectangle::~EERectangle()
{
}