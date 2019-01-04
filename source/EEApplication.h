/////////////////////////////////////////////////////////////////////
// Filename: EEApplication.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>

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

	/**
	 * Creates a MESH with the data passed in and the faces described in the indices array.
	 * In order to make a shader work with such a mes(s)h you need to set its shaderInputType
	 * to EE_SHADER_INPUT_TYPE_CUSTOM and define your own shaderInput.
	 * 
	 * @param pVertices				Void pointer for all kinds of vertex data
	 * @param amountVertices	The amount of vertices that are supposed to be in pVertices
	 * @param indices					List of the indices describing the faces
	 *
	 * @return Handle to the created mesh (nullptr if an error occured)
	 **/
	EEMesh CreateMesh(
		void const*									 pVertices,
		size_t											 amountVertices,
		std::vector<uint32_t> const& indices);

	/**
	 * Creates a BUFFER handle being able to store the passed in size of data
	 * 
	 * @param bufferSize			Desired size of the buffer in bytes
	 *
	 * @return Handle to the created buffer, that can be bound as a resource
	 **/
	EEBuffer CreateBuffer(size_t bufferSize);

	/**
	 * Creates a TEXTURE from the file passed in.
	 *
	 * @param fileName								Destination of the texture file to read in
	 * @param enableMipMapping				If set to true different mipmap levels will be generated and used
	 * @param unnormalizedCoordinates If false coordinates will be normalized to 0 to 1
	 *
	 * @return Handle to the created texture, that can be bound as a resource
	 **/
	EETexture CreateTexture(
		char const* fileName,
		EEBool32		enableMipMapping,
		EEBool32		unnormalizedCoordinates);

	/**
	 * Creates a TEXTURE corresponding to the create info passed in.
	 *
	 * @param textureCInfo	Struct holding creation infos for the desired texture
	 *
	 * @return Handle to the created texture, that can be bound as a resource
	 **/
	EETexture CreateTexture(EETextureCreateInfo const& textureCInfo);

	/**
	 * Creates a SHADER with the settings specified in the EEShaderCreateInfo. Does only use
	 * compiled shader in Spir-V (possible @TODO here).
	 *
	 * @param shaderCInfo		Struct that holds the description of the desired shader
	 *
	 * @return Handle to the shader, that can be used to create an object (nullptr if an error occured)
	 **/
	EEShader CreateShader(EEShaderCreateInfo const& shaderCInfo);

	/**
	 * Creates an OBJECT that will be rendered everytime Draw is called. The object
	 * uses the mesh pased in and renders it with the shader passed in. The splitscreen
	 * option can be ignored if splitscreen wasn't enabled.
	 *
	 * @param shader				The shader that will be used to render
	 * @param mesh					The mesh that will be rendered
	 * @param bindings			The actual resources that will be bound to the shader
	 * @param splitscreen		Which of the splitted screens should this object be rendered on (defaults)
	 *
	 * @return Handle to the created object (nullptr if an error occured)
	 **/
	EEObject CreateObject(
		EEShader																		shader,
		EEMesh																			mesh,
		std::vector<EEObjectResourceBinding> const& bindings,
		EESplitscreen																splitscreen = EE_SPLITSCREEN_UNDEFINED);

	/**
	 * Updates the buffer passed in with the data passed in
	 * 
	 * @param buffer			The buffer that is desired to be updated
	 * @param pData				Void pointer to the new data for the buffer
	 **/
	void UpdateBuffer(EEBuffer buffer, void const* pData);

private:
	static void Resize(GLFWwindow*, int, int, void*);

private:
	EE::Window* m_pWindow;
	EE::Graphics* m_pGraphics;
	bool isCreated{ false };
};