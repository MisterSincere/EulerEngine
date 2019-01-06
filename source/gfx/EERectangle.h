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
			DirectX::XMFLOAT3 color;
		};
		struct VertexUBO {
			DirectX::XMFLOAT4X4 ortho;
			DirectX::XMFLOAT4X4 baseView;
			DirectX::XMFLOAT4X4 world;
		};

	public:
		EERectangle(EEApplication* pApp, EEPoint32F const& pos = { 0.0f, 0.0f }, EERect32U const& size = { 0u, 0u });
		~EERectangle();

		void Update();

		virtual void SetBounds(EEPoint32F const& pos, EERect32U const& size) { SetPosition(pos); SetSize(size); }
		virtual void SetPositionAligned(EECenterFlags f);
		virtual void SetPosition(EEPoint32F const& pos);
		virtual void SetSize(EERect32U const& size);

	protected:
		EEApplication* i_pApp;
		EEPoint32F i_position;
		EERect32U i_size;

		EEShader m_shader;
		EEMesh m_mesh;
		EEBuffer m_vertexUniformBuffer;
		VertexUBO m_vertexUniformBufferContent;
		EEObject m_object;
	};

}