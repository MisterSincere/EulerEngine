/////////////////////////////////////////////////////////////////////
// Filename: eeObject.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "eedefs.h" //< keycodes.h

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
typedef struct VkCommandBuffer_T* VkCommandBuffer;
namespace EE {
	namespace vulkan {
		struct Renderer;
	}
}

namespace EE
{

	class Object
	{
	public:
		Object();
		~Object();


		friend vulkan::Renderer;
	private:
		void Record(VkCommandBuffer);

		bool m_is2DObject;
		EESplitscreen m_splitscreen;
	};
}