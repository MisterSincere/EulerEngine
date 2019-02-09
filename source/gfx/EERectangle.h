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
	enum EECenterFlags {
		HORIZONTAL = 0x01,
		VERTICAL	 = 0x02,
		COMPLETE	 = HORIZONTAL | VERTICAL
	};

	class EERectangle
	{
	public:
		EERectangle(EEApplication* pApp, EEPoint32F const& pos = { 0.0f, 0.0f }, EERect32U const& size = { 200u, 200u });
		EERectangle(EERectangle const&) = delete;
		EERectangle(EERectangle&&) = delete;
		virtual ~EERectangle();

		virtual void Update();

		void SetBounds(EEPoint32F const& pos, EERect32U const& size) { SetPosition(pos); SetSize(size); }
		virtual void SetPositionAligned(EECenterFlags f);
		virtual void SetPosition(EEPoint32F const& pos);
		virtual void SetSize(EERect32U const& size);
		void SetBackgroundColor(EEColor const& color);
		void EnableHover(EEColor const& color);
		void DisableHover();
		void EnableActive(EEColor const& color);
		void DisableActive();
		void SetVisibility(EEBool32 visible);
		bool Intersect(EEPoint64F const& pos);
		bool Clicked(EEMouseButton button); 

		EEPoint32F const& GetPosition();
		EERect32U const& GetSize();

		EERectangle& operator=(EERectangle const&) = delete;
		EERectangle& operator=(EERectangle&&) = delete;

	protected:
		EEApplication*	i_pApp;
		EEPoint32F			i_position;
		EERect32U				i_size;
		EERect32U				i_initialWindowExtent;
		EEColor					i_bgColor;
		EEColor					i_hoverColor;
		bool						i_hoverEnabled{ false };
		EEColor					i_activeColor;
		bool						i_activeEnabled{ false };

		uint32_t				i_changes{ 0u };

		EEObject	i_object;
		EEMesh		i_mesh;
		EEShader	i_shader;
		EEBuffer	i_vertexUniformBuffer;
		EEBuffer	i_fragmentUniformBuffer;
		EEShaderColor2D::VertexUBO	 i_vertexUniformBufferContent;
		EEShaderColor2D::FragmentUBO i_fragmentUniformBufferContent;
	};

}