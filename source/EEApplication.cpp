/////////////////////////////////////////////////////////////////////
// Filename: EEApplication.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EEApplication.h"

//////////////
// INCLUDES //
//////////////
#include <cassert>

/////////////////
// MY INCLUDES //
/////////////////
#include "Window.h"
#include "Graphics.h"


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

	isCreated = true;
	return EE_TRUE;
}

void EEApplication::Release()
{
	if(isCreated) {
		RELEASE_S(m_pGraphics);
		RELEASE_S(m_pWindow);

		isCreated = false;
	}
}

bool EEApplication::PollEvent()
{
	return m_pWindow->PollEvents();
}

void EEApplication::Draw()
{
	m_pGraphics->pRenderer->RecordDrawCommands(std::vector<EE::Object*>(0));
	m_pGraphics->pRenderer->Draw();
}

EEShader EEApplication::CreateShader(EEShaderCreateInfo const& cinfo)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create a shader without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateShader(cinfo);
}

void EEApplication::Resize(GLFWwindow* window, int w, int h, void* userData)
{

}