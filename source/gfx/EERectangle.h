/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "eedefs.h"

#define POSITION_CHANGE		0x0001
#define SIZE_CHANGE				0x0002
#define TEXT_CHANGE				0x0004
#define PADDING_CHANGE		0x0008
#define TEXTCOLOR_CHANGE	0x0010

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
class EEApplication;

namespace GFX
{
	struct EERectangleCreateInfo {
		EEPoint32F			position				{ 0.f, 0.f };
		EECenterFlags		positionFlags		{ EE_CENTER_NONE };
		EERect32F				size						{ 200.0f, 200.0f };
		EEColor					backgroundColor	{ 0.f, 0.f, 0.f, 1.f };
		bool						enableHover			{ false };
		EEColor					hoverColor			{ 0.f, 0.f, 0.f, 0.f };
		bool						enableActive		{ false };
		EEColor					activeColor			{ 0.f, 0.f, 0.f, 0.f };
		bool						visibility			{ true };
	};

	class EERectangle
	{
	public:
		EERectangle(EEApplication* pApp);
		EERectangle(EEApplication* pApp, EERectangleCreateInfo const&);
		EERectangle(EERectangle const&) = delete;
		EERectangle(EERectangle&&) = delete;
		virtual ~EERectangle();

		virtual void Update();

		void SetBounds(EEPoint32F const& pos, EERect32F const& size) { SetPosition(pos); SetSize(size); }
		virtual void SetPositionAligned(EECenterFlags f);
		virtual void SetPosition(EEPoint32F const& pos);
		virtual void SetSize(EERect32F const& size);
		void SetBackgroundColor(EEColor const& color);
		void EnableHover(EEColor const& color);
		void DisableHover();
		void EnableActive(EEColor const& color);
		void DisableActive();
		virtual void SetVisibility(bool visible);
		bool Intersect(EEPoint64F const& pos);
		bool Clicked(EEMouseButton button); 

		EEPoint32F GetPosition();
		EERect32F GetSize();

		EERectangle& operator=(EERectangle const&) = delete;
		EERectangle& operator=(EERectangle&&) = delete;

	protected:
		EEApplication*	i_pApp;
		EEPoint32F			i_position;
		EERect32F				i_size;
		EERect32U				i_initialWindowExtent;
		EEColor					i_bgColor;
		EEColor					i_hoverColor;
		bool						i_hoverEnabled;
		EEColor					i_activeColor;
		bool						i_activeEnabled;

		uint32_t				i_changes{ 0u };

		EEObject	i_object;
		EEMesh		i_mesh;
		EEShader	i_shader;
		EEBuffer	i_vertexUniformBuffer;
		EEBuffer	i_fragmentUniformBuffer;
		EEShaderColor2D::VertexUBO	 i_vertexUniformBufferContent;
		EEShaderColor2D::FragmentUBO i_fragmentUniformBufferContent;

		bool i_isCreated{ false };
	};

}