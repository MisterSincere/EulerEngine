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
	m_pGraphics->Draw();
}

EEMesh EEApplication::CreateMesh(void const* pVertices, size_t amountVertices, std::vector<uint32_t> const& indices)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create a mesh without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateMesh(pVertices, amountVertices, indices);
}

EEBuffer EEApplication::CreateBuffer(size_t bufferSize)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create a buffer without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateBuffer(bufferSize);
}

EETexture EEApplication::CreateTexture(char const* fileName, EEBool32 enableMipMapping, EEBool32 unnormalizedCoordinates)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create a texture without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateTexture(fileName, enableMipMapping, unnormalizedCoordinates);
}

EETexture EEApplication::CreateTexture(EETextureCreateInfo const & textureCInfo)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create a texture without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateTexture(textureCInfo);
}

EEShader EEApplication::CreateShader(EEShaderCreateInfo const& cinfo)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create a shader without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateShader(cinfo);
}

EEObject EEApplication::CreateObject(EEShader shader, EEMesh mesh, std::vector<EEObjectResourceBinding> const& bindings, EESplitscreen splitscreen)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to create an object without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	return m_pGraphics->CreateObject(shader, mesh, bindings, splitscreen);
}

void EEApplication::UpdateBuffer(EEBuffer buffer, void const* pData)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to update a buffer without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->UpdateBuffer(buffer, pData);
}

void EEApplication::Resize(GLFWwindow* window, int w, int h, void* userData)
{

}