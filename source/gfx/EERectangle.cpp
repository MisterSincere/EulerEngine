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

	std::vector<EEShaderInputDesc> inputDescs(2);
	inputDescs[0].location = 0u;
	inputDescs[0].format = EE_FORMAT_R32G32B32_SFLOAT;
	inputDescs[0].offset = offsetof(Vertex, position);

	inputDescs[1].location = 1u;
	inputDescs[1].format = EE_FORMAT_R32G32B32_SFLOAT;
	inputDescs[1].offset = offsetof(Vertex, color);

	EEVertexInput shaderInput;
	shaderInput.amountInputs = uint32_t(inputDescs.size());
	shaderInput.pInputDescs = inputDescs.data();
	shaderInput.inputStride = sizeof(EERectangle::Vertex);

	EEDescriptorDesc descriptor;
	descriptor.type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor.shaderStage = EE_SHADER_STAGE_VERTEX;
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

	float height = 200.0f;
	float width = 200.0f;

	std::vector<Vertex> vertices = {
		{{-width, +height, 0.0f}, {1.0f, 0.0f, 0.0f}}, //< TOP LEFT
		{{-width, -height, 0.0f}, {0.0f, 1.0f, 0.0f}}, //< BOTTOM LEFT
		{{+width, -height, 0.0f}, {0.0f, 0.0f, 1.0f}}, //< BOTTOM RIGHT
		{{+width, +height, 0.0f}, {1.0f, 0.0f, 1.0f}}, //< TOP RIGHT
	};

	std::vector<uint32_t> indices = {
		0, 1, 2,
		0, 2, 3
	};
	m_mesh = pApp->CreateMesh(vertices.data(), sizeof(Vertex) * vertices.size(), indices);


	m_vertexUniformBuffer = pApp->CreateBuffer(sizeof(VertexUBO));


	std::vector<EEObjectResourceBinding> bindings(1);
	bindings[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].binding = 0u;
	bindings[0].resource = m_vertexUniformBuffer;
	m_object = pApp->CreateObject(m_shader, m_mesh, bindings);

	VertexUBO ubo;
	DirectX::XMStoreFloat4x4(&ubo.ortho, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&ubo.baseView, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&ubo.world, DirectX::XMMatrixIdentity());
	pApp->UpdateBuffer(m_vertexUniformBuffer, &ubo);
}

EERectangle::~EERectangle()
{
}