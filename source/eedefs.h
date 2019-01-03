/////////////////////////////////////////////////////////////////////
// Filename: eedefs.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <cstdio>
#include <DirectXMath.h>

#include "keycodes.h"

//////////////////////
// DEFINES / MACROS //
//////////////////////
#define EE_TRUE 1
#define EE_FALSE 0

#define EE_DEFINE_HANDLE(name) typedef uint32_t* name;

#define EE_PRINT(val, ...) printf_s(val, __VA_ARGS__);

#define EE_INVARIANT(expr) assert(expr);

#define RELEASE_S(p) {if(p) {delete p; p = nullptr;}}
#define RELEASE_A(p) {if(p) {delete[] p; p = nullptr;}}

// std string wrap for the assets path
#define EE_ASSETS_DIR(path) (std::string(EE_CMAKE_ASSETS_PATH) + path);

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
struct GLFWwindow;

//////////////
// TYPEDEFS //
//////////////
typedef uint32_t EEFlags;
typedef uint32_t EEBool32;
typedef char const* EEcstr;
typedef char* EEstr;

/////////////
// HANDLES //
/////////////
EE_DEFINE_HANDLE(EEHandle) //< Generalization of a handle
EE_DEFINE_HANDLE(EEShader);
EE_DEFINE_HANDLE(EEMesh);
EE_DEFINE_HANDLE(EEObject);
EE_DEFINE_HANDLE(EETexture);
EE_DEFINE_HANDLE(EEBuffer);

///////////
// ENUMS //
///////////
enum EEDescriptorType {
	EE_DESCRIPTOR_TYPE_SAMPLER,
	EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER
};

enum EEFormat {
	EE_FORMAT_R32G32_SFLOAT,
	EE_FORMAT_R32G32B32_SFLOAT,
	EE_FORMAT_R8_UINT,
	EE_FORMAT_R8_UNORM,
	EE_FORMAT_R8G8B8A8_UINT,
	EE_FORMAT_R8G8B8A8_UNORM,
};

enum EERenderType {
	EE_RENDER_TYPE_2D = 0x01,
	EE_RENDER_TYPE_3D = 0x02,
	EE_RENDER_TYPE_BOTH = 0x03
};

enum EEScreenMode {
	EE_SCREEN_MODE_WINDOWED				 = 0x01,
	EE_SCREEN_MODE_FULLSCREEN			 = 0x02,
	EE_SCREEN_MODE_FAKE_FULLSCREEN = 0x04,
	EE_SCREEN_MODE_MAXIMIZED			 = 0x08
};

enum EEShaderInputType {
	EE_SHADER_INPUT_TYPE_OBJ_MESH,
	EE_SHADER_INPUT_TYPE_CUSTOM
};

enum EEShaderStage {
	EE_SHADER_STAGE_VERTEX,
	EE_SHADER_STAGE_FRAGMENT
};

enum EESplitscreen {
	EE_SPLITSCREEN_LEFT					= 0x01,
	EE_SPLITSCREEN_RIGHT				= 0x02,
	EE_SPLITSCREEN_TOP					= 0x04,
	EE_SPLITSCREEN_BOTTOM				= 0x08,
	EE_SPLITSCREEN_TOP_LEFT			= EE_SPLITSCREEN_TOP		| EE_SPLITSCREEN_LEFT,
	EE_SPLITSCREEN_TOP_RIGHT		= EE_SPLITSCREEN_TOP		| EE_SPLITSCREEN_RIGHT,
	EE_SPLITSCREEN_BOTTOM_LEFT	= EE_SPLITSCREEN_BOTTOM | EE_SPLITSCREEN_LEFT,
	EE_SPLITSCREEN_BOTTOM_RIGHT = EE_SPLITSCREEN_BOTTOM | EE_SPLITSCREEN_RIGHT,
	EE_SPLITSCREEN_UNDEFINED		// Forbidden if splitscreen mode was enabled in EEGraphicsCreateInfo
};

enum EESplitscreenMode {
	EE_SPLITSCREEN_MODE_NONE			 = 0x00,
	EE_SPLITSCREEN_MODE_VERTICAL	 = 0x01,
	EE_SPLITSCREEN_MODE_HORIZONTAL = 0x02,
	EE_SPLITSCREEN_MODE_RASTER		 = EE_SPLITSCREEN_MODE_HORIZONTAL | EE_SPLITSCREEN_MODE_VERTICAL,
};

enum EEWindowFlags {
	EE_WINDOW_FLAGS_NONE		= 0x00,
	EE_WINDOW_FLAGS_CENTERX = 0x01,
	EE_WINDOW_FLAGS_CENTERY = 0x02,
	EE_WINDOW_FLAGS_CENTER	= 0x03
};

/////////////
// STRUCTS //
/////////////
struct EEColor {
	float r;
	float g; 
	float b;
	float a;
};

struct EERect32U {
	uint32_t width;
	uint32_t height;
};

struct EEPoint32 {
	int32_t x;
	int32_t y;
};

struct EEPoint32F {
	float x;
	float y;
};

struct EEShaderInputDesc {
	uint32_t location;
	EEFormat format;
	uint32_t offset;
};

struct EEVertexInput {
	uint32_t								 amountInputs;
	EEShaderInputDesc const* pInputDescs;
	uint32_t								 inputStride;
};

struct EEDescriptorDesc {
	EEDescriptorType type;
	EEShaderStage		 shaderStage;
	uint32_t				 binding;
};

struct EEPushConstantDesc {
	EEShaderStage shaderStage;
	uint32_t			size;
	void*					pData;
};

struct EEObjectResourceBinding {
	EEDescriptorType type;
	uint32_t				 binding;
	EEHandle				 resource;
};

struct EEPredefinedVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
	DirectX::XMFLOAT2 uvCoord;
	DirectX::XMFLOAT3 normal;
};

struct EEApplicationCreateInfo {
	EEWindowFlags			flags;
	EERect32U					clientSize;
	EEPoint32					position;
	EEScreenMode			screenMode;
	char const*				title;
	char const*				icon;						// @TODO Not used yet
	EEBool32					mouseDisabled;
	EESplitscreenMode splitscreen;
	EERenderType			rendererType;
};

struct EEShaderCreateInfo {
	char const*								vertexFileName;
	char const*								fragmentFileName;
	uint32_t									amountObjects;		//< amount objects that will use this shader
	EEShaderInputType					shaderInputType;	//< if not EE_SHADER_INPUT_TYPE_CUSTOM this shader should only be used for obj file read meshes
	EEVertexInput const*			pVertexInput;
	uint32_t									amountDescriptors;
	EEDescriptorDesc const*		pDescriptors;
	EEPushConstantDesc const* pPushConstant;
	EEBool32									is2DShader;
	EEBool32									wireframe;
	EEBool32									clockwise;
};

struct EETextureCreateInfo {
	unsigned char* pData;
	EERect32U			 extent;
	EEBool32			 unnormalizedCoordinates;
	EEBool32			 enableMipMapping;
	EEBool32			 format;
};