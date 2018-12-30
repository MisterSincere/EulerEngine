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

class EEApplication {
public:
	EEApplication();
	~EEApplication();

	EEBool32 Create(EEApplicationCreateInfo const&);
	void Release();

	bool PollEvent();

private:
	static void Resize(GLFWwindow*, int, int, void*);

private:
	EE::Window* m_pWindow;
	EE::Graphics* m_pGraphics;
};