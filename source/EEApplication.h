/////////////////////////////////////////////////////////////////////
// Filename: EEApplication.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "eedefs.h"

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
	void Draw();

	EEShader CreateShader(EEShaderCreateInfo const& shaderCInfo);

private:
	static void Resize(GLFWwindow*, int, int, void*);

private:
	EE::Window* m_pWindow;
	EE::Graphics* m_pGraphics;
	bool isCreated{ false };
};