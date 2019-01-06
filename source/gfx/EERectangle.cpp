/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EERectangle.h"

#include <EEApplication.h>
#include <string>

using namespace GFX;
using namespace DirectX;

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


	std::vector<Vertex> vertices = {
		{{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, //< TOP LEFT
		{{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, //< BOTTOM LEFT
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, //< BOTTOM RIGHT
		{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}, //< TOP RIGHT
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

	// Initialize vertex buffer content
	XMStoreFloat4x4(&m_vertexUniformBufferContent.ortho, i_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&m_vertexUniformBufferContent.baseView, i_pApp->AcquireBaseViewLH());
	// Create world matrix with passed in size and position
	EERect32U wExtent = pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)size.width, (float)size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + pos.x, -(wExtent.height / 2.0f) + pos.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}

EERectangle::~EERectangle()
{
}

void EERectangle::Update()
{
	XMStoreFloat4x4(&m_vertexUniformBufferContent.ortho, i_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&m_vertexUniformBufferContent.baseView, i_pApp->AcquireBaseViewLH());
	i_pApp->UpdateBuffer(m_vertexUniformBuffer, &m_vertexUniformBufferContent);
}

void EERectangle::SetPositionAligned(EECenterFlags f) {
	EERect32U wExtent = i_pApp->GetWindowExtent();
	// Compute new position
	i_position = {
		(f&HORIZONTAL) ? (wExtent.width - i_size.width) / 2.0f : i_position.x,
		(f&VERTICAL) ? (wExtent.height - i_size.height) / 2.0f : i_position.y
	};
	// Update world matrix
	XMMATRIX world = XMMatrixScaling((float)i_size.width, (float)i_size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + i_position.x, -(wExtent.height / 2.0f) + i_position.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}

void EERectangle::SetPosition(EEPoint32F const& pos)
{
	i_position = pos;
	// Update world matrix
	EERect32U wExtent = i_pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)i_size.width, (float)i_size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + i_position.x, -(wExtent.height / 2.0f) + i_position.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}

void EERectangle::SetSize(EERect32U const& size)
{
	i_size = size;
	// Update world matrix
	EERect32U wExtent = i_pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)i_size.width, (float)i_size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + i_position.x, -(wExtent.height / 2.0f) + i_position.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}