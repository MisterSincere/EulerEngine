/////////////////////////////////////////////////////////////////////
// Filename: applicationClass.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

//#include "eedefs.h" already included within window.h
#include "windowClass.h"

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
namespace EE {
	struct Window;
	struct Graphics;
}
struct GLFWwindow;

namespace EE {

	class Application {
	public:
		Application();
		~Application();

		EEBool32 Create(EEWindowCreateInfo const&);
		void Release();

		bool PollEvent();

	private:
		static void Resize(GLFWwindow*, int, int, void*);

	private:
		Window* m_pWindow;
		Graphics* m_pGraphics;
	};

}