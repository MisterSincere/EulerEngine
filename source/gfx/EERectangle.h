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
			DirectX::XMFLOAT2 pos;
		};
		struct FragmentUBO {
			DirectX::XMFLOAT3 color;
		};

	public:
		EERectangle(EEApplication* pApp, EEPoint32F const& pos = { 0.0f, 0.0f }, EERect32U const& size = { 0u, 0u });
		~EERectangle();

		virtual void SetBounds(EEPoint32F const& pos, EERect32U const& size) { SetPosition(pos); SetSize(size); }
		virtual void SetPositionAligned(EECenterFlags f, EERect32U const& windowSize) {
			SetPosition({
				(f&HORIZONTAL) ? (windowSize.width  - i_size.width)  / 2.0f : i_position.x,
				(f&VERTICAL)	 ? (windowSize.height - i_size.height) / 2.0f : i_position.y
			});
		}
		virtual void SetPosition(EEPoint32F const& pos) { i_position = pos; }
		virtual void SetSize(EERect32U const& size) { i_size = size; }

	protected:
		EEApplication* i_pApp;
		EEPoint32F i_position;
		EERect32U i_size;

		EEShader m_shader;
		EEMesh m_mesh;
		EEBuffer m_uniformBuffer;
		EEObject m_object;
	};

}