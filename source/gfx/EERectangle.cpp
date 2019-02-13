/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EERectangle.h"

#include <string>

#include "EEApplication.h"
#include "vkcore/vulkanTools.h"

using namespace GFX;
using namespace DirectX;



EERectangle::EERectangle(EEApplication* pApp)
	: EERectangle(pApp, EERectangleCreateInfo())
{}

EERectangle::EERectangle(EEApplication* pApp, EERectangleCreateInfo const& cinfo)
	: i_pApp(pApp)
	, i_position(cinfo.position)
	, i_size(cinfo.size)
	, i_initialWindowExtent(pApp->GetWindowExtent())
	, i_bgColor(cinfo.backgroundColor)
	, i_hoverColor(cinfo.hoverColor)
	, i_hoverEnabled(cinfo.enableHover)
	, i_activeColor(cinfo.activeColor)
	, i_activeEnabled(cinfo.enableActive)
{
	if (!pApp) {
		EE_PRINT("[EERectangle] Passed in application is invalid!\n");
		return;
	}

	if (cinfo.positionFlags & GFX::HORIZONTAL) i_position.x = (i_initialWindowExtent.width - i_size.width) / 2.f;
	if (cinfo.positionFlags & GFX::VERTICAL) i_position.y = (i_initialWindowExtent.height - i_size.height) / 2.f;

	i_shader = i_pApp->AcquireShaderColor2D();

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
	i_mesh = i_pApp->CreateMesh(vertices.data(), sizeof(EEShaderColor2D::VertexInputType) * vertices.size(), indices);

	// RESOURCES
	i_vertexUniformBuffer = i_pApp->CreateBuffer(sizeof(EEShaderColor2D::VertexUBO));
	i_fragmentUniformBuffer = i_pApp->CreateBuffer(sizeof(EEShaderColor2D::FragmentUBO));

	// OBJECT
	std::vector<EEObjectResourceBinding> bindings(2);
	bindings[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].binding = 0u;
	bindings[0].resource = i_vertexUniformBuffer;

	bindings[1].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].binding = 1u;
	bindings[1].resource = i_fragmentUniformBuffer;

	i_object = i_pApp->CreateObject(i_shader, i_mesh, bindings);


	// Initialize vertex buffer content
	XMStoreFloat4x4(&i_vertexUniformBufferContent.ortho, i_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&i_vertexUniformBufferContent.baseView, i_pApp->AcquireBaseViewLH());
	// Create world matrix with passed in size and position
	XMMATRIX world = XMMatrixScaling(i_size.width, i_size.height, 1.0f);
	world *= XMMatrixTranslation(-(i_initialWindowExtent.width / 2.0f) + i_position.x,
		-(i_initialWindowExtent.height / 2.0f) + i_position.y, 0.0f);
	XMStoreFloat4x4(&i_vertexUniformBufferContent.world, world);

	// Initialize fragment buffer content
	XMStoreFloat4(&i_fragmentUniformBufferContent.fillColor, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));

	// Hide rectangle if desired
	if (!cinfo.visibility) {
		i_pApp->SetObjectVisibility(i_object, EE_FALSE);
	}

	i_isCreated = true;
}



EERectangle::~EERectangle()
{
	if (i_isCreated) {
		i_pApp->ReleaseObject(i_object);
		i_pApp->ReleaseMesh(i_mesh);

		i_isCreated = false;
	}
}










void EERectangle::Update()
{
	if (i_changes & SIZE_CHANGE || i_changes & POSITION_CHANGE) {
		// Update world matrix
		EERect32U wExtent = i_pApp->GetWindowExtent();
		XMMATRIX world = XMMatrixScaling(i_size.width, i_size.height, 1.0f);
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









void EERectangle::SetPositionAligned(EECenterFlags f)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING CHANGE POSITION YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EERect32U wExtent = i_pApp->GetWindowExtent();
	// Compute new position
	i_position = {
		(f&HORIZONTAL) ? (wExtent.width - i_size.width) / 2.0f : i_position.x,
		(f&VERTICAL) ? (wExtent.height - i_size.height) / 2.0f : i_position.y
	};
	i_changes |= POSITION_CHANGE;

	EE_INFO("[EERECTANGLE] New position: %f, %f\n", i_position.x, i_position.y);
}

void EERectangle::SetPosition(EEPoint32F const& pos)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING CHANGE POSITION YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] New position: %f, %f\n", pos.x, pos.y);
	i_position = pos;
	i_changes |= POSITION_CHANGE;
}

void EERectangle::SetSize(EERect32F const& size)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING GET POSITION YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] New size: %d, %d\n", size.width, size.height);
	i_size = size;
	i_changes |= SIZE_CHANGE;
}

void EERectangle::SetBackgroundColor(EEColor const& color)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING CHANGE BG COLOR YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] New bgcolor: %f, %f, %f, %f\n", color.r, color.g, color.b, color.a);
	i_bgColor = color;
}

void EERectangle::EnableHover(EEColor const& color)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING ENABLE HOVER YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] New hovercolor: %f, %f, %f, %f\n", color.r, color.g, color.b, color.a);
	i_hoverColor = color;
	i_hoverEnabled = true;
}

void EERectangle::DisableHover()
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING DISABLE HOVER YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] Disabled hover effect\n");
	i_hoverEnabled = false;
}

void EERectangle::EnableActive(EEColor const & color)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING ENABLE ACTIVE YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] New activecolor: %f, %f, %f, %f\n", color.r, color.g, color.b, color.a);
	i_activeColor = color;
	i_activeEnabled = true;
}

void EERectangle::DisableActive()
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING DISABLE ACTIVE YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	EE_INFO("[EERECTANGLE] Disabled active effect\n");
	i_activeEnabled = false;
}

void EERectangle::SetVisibility(bool visible)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING CHANGE VISIBILITY YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return;
	}

	i_pApp->SetObjectVisibility(i_object, visible);
}






bool EERectangle::Intersect(EEPoint64F const& pos)
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING CHECK INTERSECTION YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return false;
	}

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
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING CHECK IF CLICKED YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return false;
	}

	return i_pApp->MouseHit(button) && Intersect(i_pApp->MousePosition());
}




EEPoint32F EERectangle::GetPosition()
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING GET POSITION YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return { -1.f, -1.f };
	}

	return i_position;
}

EERect32F EERectangle::GetSize()
{
	if (!i_isCreated) {
		EE_PRINT("[EERECTANGLE] WHY BOTHER TO FUCKING GET SIZE YOU RETARD!! RECT WASNT CREATED YET!!!!!\n");
		return { -1.f, -1.f };
	}

	return i_size;
}