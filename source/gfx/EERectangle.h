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
	private:
		struct Vertex {
			DirectX::XMFLOAT3 position;
		};
		struct VertexUBO {
			DirectX::XMFLOAT4X4 ortho;
			DirectX::XMFLOAT4X4 baseView;
			DirectX::XMFLOAT4X4 world;
		};
		struct FragmentUBO {
			DirectX::XMFLOAT4 bgColor;
		};

	public:
		EERectangle(EEApplication* pApp, EEPoint32F const& pos = { 0.0f, 0.0f }, EERect32U const& size = { 0u, 0u });
		~EERectangle();

		void Update();

		void SetBounds(EEPoint32F const& pos, EERect32U const& size) { SetPosition(pos); SetSize(size); }
		void SetPositionAligned(EECenterFlags f);
		void SetPosition(EEPoint32F const& pos);
		void SetSize(EERect32U const& size);
		void SetBackgroundColor(EEColor const& color);

	private:
		EEApplication* m_pApp;
		EEPoint32F m_position;
		EERect32U m_size;

		static EEShader m_shader;
		static EEMesh m_mesh;
		EEBuffer m_vertexUniformBuffer;
		VertexUBO m_vertexUniformBufferContent;
		EEBuffer m_fragmentUniformBuffer;
		FragmentUBO m_fragmentUniformBufferContent;
		EEObject m_object;

		static bool m_fistRectangle;
	};

}