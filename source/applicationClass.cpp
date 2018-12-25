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
#include "graphicsClass.h"


EE::Application::Application()
	: m_pWindow(new EE::Window)
	, m_pGraphics(new EE::Graphics)
{

}

EE::Application::~Application()
{
	Release();
}

EEBool32 EE::Application::Create(EEWindowCreateInfo const& windowCInfo)
{
	assert(m_pWindow);
	if (!m_pWindow->Create(windowCInfo, Resize, this)) {
		return EE_FALSE;
	}

	assert(m_pGraphics);
	if (!m_pGraphics->Create(m_pWindow)) {
		return EE_FALSE;
	}


	return EE_TRUE;
}

void EE::Application::Release()
{
	RELEASE_S(m_pGraphics);
	RELEASE_S(m_pWindow);
}

bool EE::Application::PollEvent()
{
	return m_pWindow->PollEvents();
}


void EE::Application::Resize(GLFWwindow* window, int w, int h, void* userData)
{

}