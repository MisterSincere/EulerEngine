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
	void Draw(EEColor const& color = { 0.0f, 0.0f, 0.0f, 1.0f });

	/**
	 * Creates a MESH with the data passed in and the faces described in the indices array.
	 * In order to make a shader work with such a mes(s)h you need to set its shaderInputType
	 * to EE_SHADER_INPUT_TYPE_CUSTOM and define your own shaderInput.
	 * 
	 * @param pVertices				Void pointer for all kinds of vertex data
	 * @param bufferSize			Size of the vertex data in bytes
	 * @param indices					List of the indices describing the faces
	 *
	 * @return Handle to the created mesh (nullptr if an error occured)
	 **/
	EEMesh CreateMesh(
		void const*									 pVertices,
		size_t											 bufferSize,
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
	 * UPDATES the buffer passed in with the data passed in
	 * 
	 * @param buffer			The buffer that is desired to be updated
	 * @param pData				Void pointer to the new data for the buffer
	 **/
	void UpdateBuffer(EEBuffer buffer, void const* pData);

	/**
	 * UPDATES the data of a mesh.
	 *
	 * @param mesh				Handle to the mesh that should be updated
	 * @param pVertices		New vertex data
	 * @param bufferSize	Size of the vertex data
	 * @param indices			New index data
	 **/
	void UpdateMesh(
		EEMesh											 mesh,
		void const*									 pVertices,
		size_t											 bufferSize,
		std::vector<uint32_t> const& indices);

	/**
	 * If set to true the passed in object won't be rendered
	 *
	 * @param object		The object you wanna set visible or not
	 * @param visible		Boolean indicating wether the object should be rendered
	 **/
	void SetObjectVisibility(EEObject object, EEBool32 visible);

	/**
	 * RELEASES the objects data (descriptor sets for instance). Will wait till queue is idle.
	 * The shader and meshes that this object used will NOT be released.
	 *
	 * @param object		Handle to the object that should be released
	 **/
	void ReleaseObject(EEObject& object);

	/**
	 * RELEASES the mesh's data (buffer, memory...)
	 *
	 * @param mesh			Handle to the mesh that is desired to be released
	 **/
	void ReleaseMesh(EEMesh& mesh);

	/**
	 * RELEASES the shaders' data. Will NOT release the textures that were pased in for a descriptor.
	 *
	 * @param shader		Handle to the shader that is desired to be released
	 **/
	void ReleaseShader(EEShader& shader);

	/**
	 * RELEASES the textures' data. Teh data loaded from the image will be freed.
	 *
	 * @param texture		Handle to the texture that is desired to be released
	 **/
	void ReleaseTexture(EETexture& texture);

	/**
	 * RELEASES the buffers' data and its memory.
	 *
	 * @param buffer		Handle to the buffer that is desired to be released
	 **/
	void ReleaseBuffer(EEBuffer& buffer);

	/**
	 * Returns true if the key was just hit. If they key was hit the previous frame
	 * this method returns false.
	 **/
	bool KeyHit(EEKey key);

	/**
	 * Returns true if the key is currently held down.
	 **/
	bool KeyPressed(EEKey key);

	/**
	 * Stores the current mouse position in the references passed in.
	 **/
	void MousePosition(double& posX, double& posY);

	/**
	 * Stores the movement of the mouse since the previous frame in
	 * the references passed in.
	 **/
	void MouseMovement(double& dX, double& dY);
	
	/**
	 * Returns an orthogonal left handed projection matrix for 2d rendering with the current
	 * window's extent.
	 **/
	DirectX::XMMATRIX AcquireOrthoMatrixLH();

	/**
	 * Returns an orthogonal right handed projection matrix for 2d rendering with the current
	 * window's extent.
	 **/
	DirectX::XMMATRIX AcquireOrthoMatrixRH();

	/**
	 * Returns a perspective left handed projection matrix for 3d rendering with the current
	 * window's extent.
	 **/
	DirectX::XMMATRIX AcquirePerspectiveMatrixLH();

	/**
	 * Returns a perspective right handed projection matrix for 3d rendering with the current
	 * window's extent.
	 **/
	DirectX::XMMATRIX AcquirePerspectiveMatrixRH();

	/**
	 * Returns a left handed base view matrix
	 **/
	DirectX::XMMATRIX AcquireBaseViewLH();

	/**
	 * Returns a right handed base view matrix
	 **/
	DirectX::XMMATRIX AcquireBaseViewRH();

	/**
	 * Returns a handle for the prepared color 2d shader.
	 * For vertex input and ubo types check namespace EEShaderColor2D
	 **/
	EEShader AcquireShaderColor2D();

	/**
	 * Returns the current size of the actual drawing field
	 **/
	EERect32U GetWindowExtent();

private:
	EE::Window* m_pWindow;
	EE::Graphics* m_pGraphics;
	bool isCreated{ false };
};