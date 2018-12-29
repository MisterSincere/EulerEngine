/////////////////////////////////////////////////////////////////////
// Filename: eedefs.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>

#include "keycodes.h"

//////////////////////
// DEFINES / MACROS //
//////////////////////
#define EE_TRUE 1
#define EE_FALSE 0

#define EE_PRINT(val, ...) printf_s(val, __VA_ARGS__);

#define RELEASE_S(p) {if(p) {delete p; p = nullptr;}}
#define RELEASE_A(p) {if(p) {delete[] p; p = nullptr;}}

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

///////////
// ENUMS //
///////////
enum EEWindowFlagBits {
	EE_WINDOW_FLAGS_NONE		= 0x00,
	EE_WINDOW_FLAGS_CENTERX = 0x01,
	EE_WINDOW_FLAGS_CENTERY = 0x02,
	EE_WINDOW_FLAGS_CENTER	= 0x03
};
typedef EEFlags EEWindowFlags;

enum EEScreenMode {
	EE_SCREEN_MODE_WINDOWED				 = 0x01,
	EE_SCREEN_MODE_FULLSCREEN			 = 0x02,
	EE_SCREEN_MODE_FAKE_FULLSCREEN = 0x04,
	EE_SCREEN_MODE_MAXIMIZED			 = 0x08
};

enum EESplitscreenMode {
	EE_SPLITSCREEN_MODE_NONE			 = 0x00,
	EE_SPLITSCREEN_MODE_VERTICAL	 = 0x01,
	EE_SPLITSCREEN_MODE_HORIZONTAL = 0x02,
	EE_SPLITSCREEN_MODE_RASTER		 = EE_SPLITSCREEN_MODE_HORIZONTAL | EE_SPLITSCREEN_MODE_VERTICAL,
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

enum RendererType {
	EE_RENDERER_2D = 0x01,
	EE_RENDERER_3D = 0x02
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

struct EEWindowCreateInfo {
	EEWindowFlags	flags;
	EERect32U			clientSize;
	EEPoint32			position;
	EEScreenMode	screenMode;
	char const*		title;
	char const*		icon;						// @TODO Not used yet
	EEBool32			mouseDisabled;
};

struct EEGraphicsCreateInfo {
	EESplitscreenMode splitscreen;
	RendererType			rendererType;
};