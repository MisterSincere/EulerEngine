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


EEApplication::EEApplication()
	: m_pWindow(new EE::Window)
	, m_pGraphics(new EE::Graphics)
{

}

EEApplication::~EEApplication()
{
	Release();
}

EEBool32 EEApplication::Create(EEApplicationCreateInfo const& appCInfo)
{
	assert(m_pWindow);
	if (!m_pWindow->Create(appCInfo, Resize, this)) {
		return EE_FALSE;
	}

	assert(m_pGraphics);
	if (!m_pGraphics->Create(m_pWindow, appCInfo)) {
		return EE_FALSE;
	}


	return EE_TRUE;
}

void EEApplication::Release()
{
	RELEASE_S(m_pGraphics);
	RELEASE_S(m_pWindow);
}

bool EEApplication::PollEvent()
{
	return m_pWindow->PollEvents();
}


void EEApplication::Resize(GLFWwindow* window, int w, int h, void* userData)
{

}