/////////////////////////////////////////////////////////////////////
// Filename: applicationClass.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "applicationClass.h"

//////////////
// INCLUDES //
//////////////
#include <cassert>

/////////////////
// MY INCLUDES //
/////////////////
#include "windowClass.h"


EE::Application::Application()
	: m_pWindow(new EE::Window)
{

}

EE::Application::~Application()
{
	Release();
}

EEBool32 EE::Application::Create(EEWindowCreateInfo const& windowCInfo)
{
	assert(m_pWindow);
	if (m_pWindow->CreateWindow(windowCInfo, Resize, this)) {
		return EE_FALSE;
	}


	return EE_TRUE;
}

void EE::Application::Release()
{
	RELEASE_S(m_pWindow);
}

bool EE::Application::PollEvent()
{
	return m_pWindow->PollEvents();
}


void EE::Application::Resize(GLFWwindow* window, int w, int h, void* userData)
{

}