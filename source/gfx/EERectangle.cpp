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


bool EERectangle::m_fistRectangle{ true };
EEShader EERectangle::m_shader = nullptr;
EEMesh EERectangle::m_mesh = nullptr;

EERectangle::EERectangle(EEApplication* pApp, EEPoint32F const& pos /*= { 0.0f, 0.0f }*/, EERect32U const& size /*= { 0.0f, 0.0f }*/)
	: m_pApp(pApp)
	, m_position(pos)
	, m_size(size)
{
	if (!pApp) {
		EE_PRINT("[EERectangle] Passed in application is invalid!\n");
		return;
	}

	// Create the shader and mesh if this is the first rectangle
	// other will then reuse this shader and mesh
	if (m_fistRectangle) {
		std::vector<EEShaderInputDesc> inputDescs(1);
		inputDescs[0].location = 0u;
		inputDescs[0].format = EE_FORMAT_R32G32B32_SFLOAT;
		inputDescs[0].offset = offsetof(Vertex, position);

		EEVertexInput shaderInput;
		shaderInput.amountInputs = uint32_t(inputDescs.size());
		shaderInput.pInputDescs = inputDescs.data();
		shaderInput.inputStride = sizeof(EERectangle::Vertex);

		std::vector<EEDescriptorDesc> descriptors(2);
		descriptors[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptors[0].shaderStage = EE_SHADER_STAGE_VERTEX;
		descriptors[0].binding = 0u;

		descriptors[1].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptors[1].shaderStage = EE_SHADER_STAGE_FRAGMENT;
		descriptors[1].binding = 1u;

		std::string vert = EE_ASSETS_DIR("shader/color2DVert.spv");
		std::string frag = EE_ASSETS_DIR("shader/color2DFrag.spv");
		EEShaderCreateInfo shaderCInfo;
		shaderCInfo.vertexFileName = vert.c_str();
		shaderCInfo.fragmentFileName = frag.c_str();
		shaderCInfo.amountObjects = 200u;
		shaderCInfo.shaderInputType = EE_SHADER_INPUT_TYPE_CUSTOM;
		shaderCInfo.pVertexInput = &shaderInput;
		shaderCInfo.amountDescriptors = uint32_t(descriptors.size());
		shaderCInfo.pDescriptors = descriptors.data();
		shaderCInfo.pPushConstant = nullptr;
		shaderCInfo.is2DShader = EE_TRUE;
		shaderCInfo.wireframe = EE_FALSE;
		shaderCInfo.clockwise = EE_TRUE;
		m_shader = pApp->CreateShader(shaderCInfo);

		std::vector<Vertex> vertices = {
			{{0.0f, 1.0f, 0.0f}}, //< TOP LEFT
			{{0.0f, 0.0f, 0.0f}}, //< BOTTOM LEFT
			{{1.0f, 0.0f, 0.0f}}, //< BOTTOM RIGHT
			{{1.0f, 1.0f, 0.0f}}, //< TOP RIGHT
		};
		std::vector<uint32_t> indices = {
			0, 1, 2,
			0, 2, 3
		};
		m_mesh = pApp->CreateMesh(vertices.data(), sizeof(Vertex) * vertices.size(), indices);

		// Now that the shader/mesh was created for the first time indicate that for the next creations
		m_fistRectangle = false;
	}

	// RESOURCES
	m_vertexUniformBuffer = pApp->CreateBuffer(sizeof(VertexUBO));
	m_fragmentUniformBuffer = pApp->CreateBuffer(sizeof(FragmentUBO));

	// OBJECT
	std::vector<EEObjectResourceBinding> bindings(2);
	bindings[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].binding = 0u;
	bindings[0].resource = m_vertexUniformBuffer;

	bindings[1].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].binding = 1u;
	bindings[1].resource = m_fragmentUniformBuffer;

	m_object = pApp->CreateObject(m_shader, m_mesh, bindings);


	// Initialize vertex buffer content
	XMStoreFloat4x4(&m_vertexUniformBufferContent.ortho, m_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&m_vertexUniformBufferContent.baseView, m_pApp->AcquireBaseViewLH());
	// Create world matrix with passed in size and position
	EERect32U wExtent = pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)size.width, (float)size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + pos.x, -(wExtent.height / 2.0f) + pos.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);

	// Initialize fragment buffer content
	XMStoreFloat4(&m_fragmentUniformBufferContent.bgColor, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
}

EERectangle::~EERectangle()
{
}

void EERectangle::Update()
{
	// Update the uniform buffers
	XMStoreFloat4x4(&m_vertexUniformBufferContent.ortho, m_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&m_vertexUniformBufferContent.baseView, m_pApp->AcquireBaseViewLH());
	m_pApp->UpdateBuffer(m_vertexUniformBuffer, &m_vertexUniformBufferContent);

	m_pApp->UpdateBuffer(m_fragmentUniformBuffer, &m_fragmentUniformBufferContent);
}

void EERectangle::SetPositionAligned(EECenterFlags f) {
	EERect32U wExtent = m_pApp->GetWindowExtent();
	// Compute new position
	m_position = {
		(f&HORIZONTAL) ? (wExtent.width - m_size.width) / 2.0f : m_position.x,
		(f&VERTICAL) ? (wExtent.height - m_size.height) / 2.0f : m_position.y
	};
	// Update world matrix
	XMMATRIX world = XMMatrixScaling((float)m_size.width, (float)m_size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + m_position.x, -(wExtent.height / 2.0f) + m_position.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}

void EERectangle::SetPosition(EEPoint32F const& pos)
{
	m_position = pos;
	// Update world matrix
	EERect32U wExtent = m_pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)m_size.width, (float)m_size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + m_position.x, -(wExtent.height / 2.0f) + m_position.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}

void EERectangle::SetSize(EERect32U const& size)
{
	m_size = size;
	// Update world matrix
	EERect32U wExtent = m_pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)m_size.width, (float)m_size.height, 1.0f);
	world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + m_position.x, -(wExtent.height / 2.0f) + m_position.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);
}

void EERectangle::SetBackgroundColor(EEColor const& color)
{
	m_fragmentUniformBufferContent.bgColor = { color.r, color.g, color.b, color.a };
}