/////////////////////////////////////////////////////////////////////
// Filename: eedefs.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <DirectXMath.h>

#include "keycodes.h"

//////////////////////
// DEFINES / MACROS //
//////////////////////
#define EE_TRUE 1
#define EE_FALSE 0

#define EE_DEFINE_HANDLE(name) typedef uint32_t* name;

// Avoid duplicate text wrapping definition, not too bad since they are equivalent by convention
#if defined(STR)
# undef STR
#endif

// wide- and ansi-cstring print to stdout wrapper
#define EE_PRINTA(val, ...) printf_s(val, __VA_ARGS__);
#define EE_PRINTW(val, ...) wprintf_s(L##val, __VA_ARGS__);

#if defined(UNICODE) | defined(EE_USE_UNICODE)
# define STR(str)											L##str
# define __WS													%s
# define __AS													%S
# define EE_PRINT(val, ...)						EE_PRINTW(val, __VA_ARGS__)
# define EE_STRCMP(str1, str2)				wcscmp(str1, str2)
# define EE_STRNCMP(str1, str2, len)	wcsncmp(str1, str2, len)
# define EE_STRLEN(str)								wcslen(str)
#else
# define STR(str)											str
# define __WS													%S
# define __AS													%s
# define EE_PRINT(val, ...)						EE_PRINTA(val, __VA_ARGS__)
# define EE_STRCMP(str1, str2)				strcmp(str1, str2)
# define EE_STRNCMP(str1, str2, len)	strncmp(str1, str2, len)
# define EE_STRLEN(str)								strlen(str)
#endif

/* @brief Prints informations if EE_PRINT_INFORMATIONS is defined */
#if defined(EE_PRINT_INFORMATIONS)
# define EE_INFO(val, ...) EE_PRINT(val, __VA_ARGS__)
#else
# define EE_INFO(val, ...)
#endif

#if defined(EE_PRINT_ERRORS)
# define EE_ERROR(val, ...) EE_PRINT(val, __VA_ARGS__)
#else
# define EE_ERROR(val, ...)
#endif

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
#if defined(UNICODE) | defined(EE_USE_UNICODE)
typedef wchar_t				EEchar;
typedef std::wstring	EEstring;
#else
typedef char					EEchar;
typedef std::string		EEstring;
#endif
typedef EEchar*				EEstr;
typedef EEchar const* EEcstr;

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
enum EECenterFlags {
	EE_CENTER_NONE				= 0x00,
	EE_CENTER_HORIZONTAL	= 0x01,
	EE_CENTER_VERTICAL		= 0x02,
	EE_CENTER_COMPLETE		= EE_CENTER_HORIZONTAL | EE_CENTER_VERTICAL
};

enum EEDescriptorType {
	EE_DESCRIPTOR_TYPE_SAMPLER,
	EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER
};

enum EEFormat {
	EE_FORMAT_R32G32_SFLOAT,
	EE_FORMAT_R32G32B32_SFLOAT,
	EE_FORMAT_R8_UINT,
	EE_FORMAT_R8_UNORM,
	EE_FORMAT_R8_SNORM,
	EE_FORMAT_R8G8B8A8_UINT,
	EE_FORMAT_R8G8B8A8_UNORM,
};

enum EEMouseButton {
	EE_MOUSE_BUTTON_RIGHT		= 0x01,
	EE_MOUSE_BUTTON_MIDDLE	= 0x02,
	EE_MOUSE_BUTTON_LEFT		= 0x04,
};

enum EERenderType {
	EE_RENDER_TYPE_2D		= 0x01,
	EE_RENDER_TYPE_3D		= 0x02,
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
	EE_SPLITSCREEN_UNDEFINED		//< Forbidden if splitscreen mode was enabled in EEGraphicsCreateInfo
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
	float r, g, b, a;
};

struct EERect32U {
	uint32_t width, height;
};

struct EERect64U {
	uint64_t width, height;
};

struct EERect32F {
	float width, height;
};

struct EERect64F {
	double width, height;
};

struct EEPoint32 {
	int32_t x, y;
};

struct EEPoint64 {
	int64_t x, y;
};

struct EEPoint32F {
	float x, y;
};

struct EEPoint64F {
	double x, y;
};

struct EEPadding {
	float left, top, right, bottom;
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
	char const*								geometryFileName;	//< TODO
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


namespace EEShaderColor2D {

	struct VertexInputType {
		DirectX::XMFLOAT3 position; // location = 0
	};
	// binding = 0
	struct VertexUBO {
		DirectX::XMFLOAT4X4 ortho;
		DirectX::XMFLOAT4X4 baseView;
		DirectX::XMFLOAT4X4 world;
	};
	// binding = 1
	struct FragmentUBO {
		DirectX::XMFLOAT4 fillColor;
	};

}