/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EERectangle.h"

#include <EEApplication.h>
#include <string>

#include "vkcore/vulkanTools.h"

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

	i_shader = pApp->AcquireShaderColor2D();

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
	i_mesh = pApp->CreateMesh(vertices.data(), sizeof(EEShaderColor2D::VertexInputType) * vertices.size(), indices);

	// RESOURCES
	i_vertexUniformBuffer = pApp->CreateBuffer(sizeof(EEShaderColor2D::VertexUBO));
	i_fragmentUniformBuffer = pApp->CreateBuffer(sizeof(EEShaderColor2D::FragmentUBO));

	// OBJECT
	std::vector<EEObjectResourceBinding> bindings(2);
	bindings[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].binding = 0u;
	bindings[0].resource = i_vertexUniformBuffer;

	bindings[1].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].binding = 1u;
	bindings[1].resource = i_fragmentUniformBuffer;

	i_object = pApp->CreateObject(i_shader, i_mesh, bindings);


	// Initialize vertex buffer content
	XMStoreFloat4x4(&i_vertexUniformBufferContent.ortho, i_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&i_vertexUniformBufferContent.baseView, i_pApp->AcquireBaseViewLH());
	// Create world matrix with passed in size and position
	i_initialWindowExtent = pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixScaling((float)size.width, (float)size.height, 1.0f);
	world *= XMMatrixTranslation(-(i_initialWindowExtent.width / 2.0f) + pos.x, -(i_initialWindowExtent.height / 2.0f) + pos.y, 0.0f);
	XMStoreFloat4x4(&i_vertexUniformBufferContent.world, world);

	// Initialize fragment buffer content
	XMStoreFloat4(&i_fragmentUniformBufferContent.fillColor, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
}

EERectangle::~EERectangle()
{
	i_pApp->ReleaseObject(i_object);
	i_pApp->ReleaseMesh(i_mesh);
}

void EERectangle::Update()
{
	if (i_changes & SIZE_CHANGE || i_changes & POSITION_CHANGE) {
		// Update world matrix
		EERect32U wExtent = i_pApp->GetWindowExtent();
		XMMATRIX world = XMMatrixScaling((float)i_size.width, (float)i_size.height, 1.0f);
		world *= XMMatrixTranslation(-(wExtent.width / 2.0f) + i_position.x, -(wExtent.height / 2.0f) + i_position.y, 0.0f);
		XMStoreFloat4x4(&i_vertexUniformBufferContent.world, world);
	}

	// Update the vertex uniform buffer
	XMStoreFloat4x4(&i_vertexUniformBufferContent.ortho, i_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&i_vertexUniformBufferContent.baseView, i_pApp->AcquireBaseViewLH());
	i_pApp->UpdateBuffer(i_vertexUniformBuffer, &i_vertexUniformBufferContent);

	// Default background color that can be overwritten if rectangle is active/hovered
	i_fragmentUniformBufferContent.fillColor = { i_bgColor.r, i_bgColor.g , i_bgColor.b , i_bgColor.a };

	// Check for hover, overwrites default color
	if (i_hoverEnabled && Intersect(i_pApp->MousePosition())) {
		i_fragmentUniformBufferContent.fillColor = { i_hoverColor.r, i_hoverColor.g, i_hoverColor.b, i_hoverColor.a };
	}
	// Check for active, overwrites hover/default color
	if (i_activeEnabled && i_pApp->MouseDown(EE_MOUSE_BUTTON_LEFT) && Intersect(i_pApp->MousePosition())) {
		i_fragmentUniformBufferContent.fillColor = { i_activeColor.r, i_activeColor.g, i_activeColor.b, i_activeColor.a };
	}
	// Update fragment uniform buffer storing the background color
	i_pApp->UpdateBuffer(i_fragmentUniformBuffer, &i_fragmentUniformBufferContent);

	// Reset changes, since all are uploaded now
	i_changes = 0u;
}

void EERectangle::SetPositionAligned(EECenterFlags f) {
	EERect32U wExtent = i_pApp->GetWindowExtent();
	// Compute new position
	i_position = {
		(f&HORIZONTAL) ? (wExtent.width - i_size.width) / 2.0f : i_position.x,
		(f&VERTICAL) ? (wExtent.height - i_size.height) / 2.0f : i_position.y
	};
	i_changes |= POSITION_CHANGE;
}

void EERectangle::SetPosition(EEPoint32F const& pos)
{
	i_position = pos;
	i_changes |= POSITION_CHANGE;
}

EEPoint32F const& GFX::EERectangle::GetPosition()
{
	return i_position;
}

EERect32U const& GFX::EERectangle::GetSize()
{
	return i_size;
}

void EERectangle::SetSize(EERect32U const& size)
{
	EE_INFO("[EERECTANGLE] New size: %d, %d\n", size.width, size.height);
	i_size = size;
	i_changes |= SIZE_CHANGE;
}

void EERectangle::SetBackgroundColor(EEColor const& color)
{
	i_bgColor = color;
}

void EERectangle::EnableHover(EEColor const& color)
{
	i_hoverColor = color;
	i_hoverEnabled = true;
}

void EERectangle::DisableHover()
{
	i_hoverEnabled = false;
}

void EERectangle::EnableActive(EEColor const & color)
{
	i_activeColor = color;
	i_activeEnabled = true;
}

void EERectangle::DisableActive()
{
	i_activeEnabled = false;
}

void EERectangle::SetVisibility(bool visible)
{
	i_pApp->SetObjectVisibility(i_object, visible);
}

bool EERectangle::Intersect(EEPoint64F const& pos)
{
	EERect32U curExtent = i_pApp->GetWindowExtent();
	double scaleX = (double)curExtent.width / i_initialWindowExtent.width;
	double scaleY = (double)curExtent.height / i_initialWindowExtent.height;
	double t1 = pos.x - i_position.x;
	double t2 = pos.y - i_position.y;
	return t1 > 0 && t1 < i_size.width * scaleX
		&& t2 > 0 && t2 < i_size.height * scaleY;
}

bool EERectangle::Clicked(EEMouseButton button)
{
	return i_pApp->MouseHit(button) && Intersect(i_pApp->MousePosition());
}
