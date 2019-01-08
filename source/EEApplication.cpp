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

using namespace DirectX;

/* @brief Foreward declaration of the high level resize method and definition of the user data we pass in */
void Resize(GLFWwindow* window, int w, int h, void* userData);


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
	if (!m_pWindow->Create(appCInfo, Resize, m_pGraphics)) {
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

void EEApplication::UpdateMesh(EEMesh mesh, void const* pVertices, size_t bufferSize, std::vector<uint32_t> const & indices)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to update a mesh without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->UpdateMesh(mesh, pVertices, bufferSize, indices);
}

void EEApplication::ReleaseObject(EEObject& object)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to release an object without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->ReleaseObject(object);
}

void EEApplication::ReleaseMesh(EEMesh& mesh)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to release a mesh without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->ReleaseMesh(mesh);
}

void EEApplication::ReleaseShader(EEShader& shader)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to release a shader without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->ReleaseShader(shader);
}

void EEApplication::ReleaseTexture(EETexture& texture)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to release a texture without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->ReleaseTexture(texture);
}

void EEApplication::ReleaseBuffer(EEBuffer& buffer)
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to creareleasete a buffer without a created application...!\n");
		EE_INVARIANT(isCreated);
	}
	m_pGraphics->ReleaseBuffer(buffer);
}

bool EEApplication::KeyHit(EEKey key)
{
	return m_pWindow->input.keysHit[key];
}

bool EEApplication::KeyPressed(EEKey key)
{
	return m_pWindow->input.keysPressed[key];
}

void EEApplication::MousePosition(double & posX, double & posY)
{
	posX = m_pWindow->input.mouseX;
	posY = m_pWindow->input.mouseY;
}

void EEApplication::MouseMovement(double & dX, double & dY)
{
	dX = m_pWindow->input.mouseXDelta;
	dX = m_pWindow->input.mouseXDelta;
}

DirectX::XMMATRIX EEApplication::AcquireOrthoMatrixLH()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get a drawing matrix without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return XMLoadFloat4x4(&(m_pGraphics->matrices.orthoLH));
}

DirectX::XMMATRIX EEApplication::AcquireOrthoMatrixRH()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get a drawing matrix without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return XMLoadFloat4x4(&(m_pGraphics->matrices.orthoRH));
}

DirectX::XMMATRIX EEApplication::AcquirePerspectiveMatrixLH()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get a drawing matrix without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return XMLoadFloat4x4(&(m_pGraphics->matrices.projLH));
}

DirectX::XMMATRIX EEApplication::AcquirePerspectiveMatrixRH()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get a drawing matrix without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return XMLoadFloat4x4(&(m_pGraphics->matrices.projRH));
}

DirectX::XMMATRIX EEApplication::AcquireBaseViewLH()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get a drawing matrix without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return XMLoadFloat4x4(&(m_pGraphics->matrices.baseViewLH));
}

DirectX::XMMATRIX EEApplication::AcquireBaseViewRH()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get a drawing matrix without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return XMLoadFloat4x4(&(m_pGraphics->matrices.baseViewRH));
}

EEShader EEApplication::AcquireShaderColor2D()
{
	if (!m_pGraphics->shader.color2D) {
		std::vector<EEShaderInputDesc> inputDescs(1);
		inputDescs[0].location = 0u;
		inputDescs[0].format = EE_FORMAT_R32G32B32_SFLOAT;
		inputDescs[0].offset = offsetof(EEShaderColor2D::VertexInputType, position);

		EEVertexInput shaderInput;
		shaderInput.amountInputs = uint32_t(inputDescs.size());
		shaderInput.pInputDescs = inputDescs.data();
		shaderInput.inputStride = sizeof(EEShaderColor2D::VertexInputType);

		std::vector<EEDescriptorDesc> descriptors(2);
		descriptors[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptors[0].shaderStage = EE_SHADER_STAGE_VERTEX;
		descriptors[0].binding = 0u;

		descriptors[1].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptors[1].shaderStage = EE_SHADER_STAGE_FRAGMENT;
		descriptors[1].binding = 1u;

		std::string vert = EE_ASSETS_DIR("shader/color2DVert.spv");
		std::string frag = EE_ASSETS_DIR("shader/color2DFrag.spv");
		EEShaderCreateInfo shaderCInfo;
		shaderCInfo.vertexFileName = vert.c_str();
		shaderCInfo.fragmentFileName = frag.c_str();
		shaderCInfo.amountObjects = 200u;
		shaderCInfo.shaderInputType = EE_SHADER_INPUT_TYPE_CUSTOM;
		shaderCInfo.pVertexInput = &shaderInput;
		shaderCInfo.amountDescriptors = uint32_t(descriptors.size());
		shaderCInfo.pDescriptors = descriptors.data();
		shaderCInfo.pPushConstant = nullptr;
		shaderCInfo.is2DShader = EE_TRUE;
		shaderCInfo.wireframe = EE_FALSE;
		shaderCInfo.clockwise = EE_TRUE;
		m_pGraphics->shader.color2D = CreateShader(shaderCInfo);
	}
	return m_pGraphics->shader.color2D;
}

EERect32U EEApplication::GetWindowExtent()
{
	if (!isCreated) {
		EE_PRINT("[EEAPPLICATION] Tried to get client size of window, without creating the application!\n");
		EE_INVARIANT(isCreated);
	}
	return { m_pGraphics->pSwapchain->settings.extent.width,m_pGraphics->pSwapchain->settings.extent.height };
}

void Resize(GLFWwindow* window, int w, int h, void* userData)
{
	if (w <= 0 || h <= 0) return;

	// Get the user data
	EE::Graphics* pGraphics = reinterpret_cast<EE::Graphics*>(userData);

	// Store width as unsigned int
	uint32_t width = uint32_t(w);
	uint32_t height = uint32_t(h);

	if (pGraphics) pGraphics->Resize();
}