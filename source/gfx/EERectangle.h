/////////////////////////////////////////////////////////////////////
// Filename: EERectangle.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "eedefs.h"

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
		EERectangle(EEApplication* pApp, EEPoint32F const& pos = { 0.0f, 0.0f }, EERect32U const& size = { 0u, 0u });
		~EERectangle();

		void Update();

		void SetBounds(EEPoint32F const& pos, EERect32U const& size) { SetPosition(pos); SetSize(size); }
		void SetPositionAligned(EECenterFlags f);
		void SetPosition(EEPoint32F const& pos);
		void SetSize(EERect32U const& size);
		void SetBackgroundColor(EEColor const& color);
		void SetVisibility(EEBool32 visible);
		bool Intersect(EEPoint32F const& pos);

	private:
		EEApplication* m_pApp;
		EEPoint32F m_position;
		EERect32U m_size;
		EERect32U m_initialWindowExtent;

		EEShader m_shader;
		EEMesh m_mesh;
		EEBuffer m_vertexUniformBuffer;
		EEShaderColor2D::VertexUBO m_vertexUniformBufferContent;
		EEBuffer m_fragmentUniformBuffer;
		EEShaderColor2D::FragmentUBO m_fragmentUniformBufferContent;
		EEObject m_object;
	};

}