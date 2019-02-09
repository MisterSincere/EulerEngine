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
	: m_pApp(pApp)
	, m_position(pos)
	, m_size(size)
{
	if (!pApp) {
		EE_PRINT("[EERectangle] Passed in application is invalid!\n");
		return;
	}

	m_shader = pApp->AcquireShaderColor2D();

	std::vector<EEShaderColor2D::VertexInputType> vertices = {
		{{0.0f, 1.0f, 0.0f}}, //< TOP LEFT
		{{0.0f, 0.0f, 0.0f}}, //< BOTTOM LEFT
		{{1.0f, 0.0f, 0.0f}}, //< BOTTOM RIGHT
		{{1.0f, 1.0f, 0.0f}}, //< TOP RIGHT
	};
	std::vector<uint32_t> indices = {
		0, 1, 2,
		0, 2, 3
	};
	m_mesh = pApp->CreateMesh(vertices.data(), sizeof(EEShaderColor2D::VertexInputType) * vertices.size(), indices);

	// RESOURCES
	m_vertexUniformBuffer = pApp->CreateBuffer(sizeof(EEShaderColor2D::VertexUBO));
	m_fragmentUniformBuffer = pApp->CreateBuffer(sizeof(EEShaderColor2D::FragmentUBO));

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
	m_initialWindowExtent = pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)size.width, (float)size.height, 1.0f);
	world *= XMMatrixTranslation(-(m_initialWindowExtent.width / 2.0f) + pos.x, -(m_initialWindowExtent.height / 2.0f) + pos.y, 0.0f);
	XMStoreFloat4x4(&m_vertexUniformBufferContent.world, world);

	// Initialize fragment buffer content
	XMStoreFloat4(&m_fragmentUniformBufferContent.fillColor, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
}

EERectangle::~EERectangle()
{
	m_pApp->ReleaseObject(m_object);
	m_pApp->ReleaseMesh(m_mesh);
}

void EERectangle::Update()
{
	// Update the uniform buffers
	XMStoreFloat4x4(&m_vertexUniformBufferContent.ortho, m_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&m_vertexUniformBufferContent.baseView, m_pApp->AcquireBaseViewLH());
	m_pApp->UpdateBuffer(m_vertexUniformBuffer, &m_vertexUniformBufferContent);

	// Default background color that can be overwritten if rectangle is active/hovered
	m_fragmentUniformBufferContent.fillColor = { m_bgColor.r, m_bgColor.g , m_bgColor.b , m_bgColor.a };

	// Check for hover, overwrites default color
	if (m_hoverEnabled && Intersect(m_pApp->MousePosition())) {
		m_fragmentUniformBufferContent.fillColor = { m_hoverColor.r, m_hoverColor.g, m_hoverColor.b, m_hoverColor.a };
	}
	// Check for active, overwrites hover/default color
	if (m_activeEnabled && m_pApp->MouseDown(EE_MOUSE_BUTTON_LEFT) && Intersect(m_pApp->MousePosition())) {
		m_fragmentUniformBufferContent.fillColor = { m_activeColor.r, m_activeColor.g, m_activeColor.b, m_activeColor.a };
	}
		
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
	m_bgColor = color;
}

void EERectangle::EnableHover(EEColor const& color)
{
	m_hoverEnabled = true;
	m_hoverColor = color;
}

void EERectangle::DisableHover()
{
	m_hoverEnabled = false;
}

void EERectangle::EnableActive(EEColor const & color)
{
	m_activeEnabled = true;
	m_activeColor = color;
}

void EERectangle::DisableActive()
{
	m_activeEnabled = false;
}

void EERectangle::SetVisibility(EEBool32 visible)
{
	m_pApp->SetObjectVisibility(m_object, visible);
}

bool EERectangle::Intersect(EEPoint32F const& pos)
{
	EERect32U currentExtent = m_pApp->GetWindowExtent();
	float scaleX = (float)currentExtent.width / m_initialWindowExtent.width;
	float scaleY = (float)currentExtent.height / m_initialWindowExtent.height;
	float t1 = pos.x - m_position.x;
	float t2 = pos.y - m_position.y;
	return t1 > 0.0f && t1 < m_size.width * scaleX
				&& t2 > 0.0f && t2 < m_size.height * scaleY;
}

bool EERectangle::Intersect(EEPoint64F const& pos)
{
	EERect32U curExtent = m_pApp->GetWindowExtent();
	double scaleX = (double)curExtent.width / m_initialWindowExtent.width;
	double scaleY = (double)curExtent.height / m_initialWindowExtent.height;
	double t1 = pos.x - m_position.x;
	double t2 = pos.y - m_position.y;
	return t1 > 0 && t1 < m_size.width * scaleX
		&& t2 > 0 && t2 < m_size.height * scaleY;
}

bool EERectangle::Clicked(EEMouseButton button)
{
	return m_pApp->MouseHit(button) && Intersect(m_pApp->MousePosition());
}
