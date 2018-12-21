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
	EE_WINDOW_FLAGS_NONE = 0x00,
	EE_WINDOW_FLAGS_CENTERX = 0x01,
	EE_WINDOW_FLAGS_CENTERY = 0x02,
	EE_WINDOW_FLAGS_CENTER = 0x03
};
typedef EEFlags EEWindowFlags;

enum EEScreenMode {
	EE_SCREEN_MODE_WINDOWED = 0x01,
	EE_SCREEN_MODE_FULLSCREEN = 0x02,
	EE_SCREEN_MODE_FAKE_FULLSCREEN = 0x04,
	EE_SCREEN_MODE_MAXIMIZED = 0x08
};

/////////////
// STRUCTS //
/////////////
struct EERect32U {
	uint32_t width;
	uint32_t height;
};

struct EEPoint32 {
	int32_t x;
	int32_t y;
};

struct EEWindow {
	EERect32U			clientSize;
	EEPoint32			position;
	GLFWwindow*		winHandle;
	EEScreenMode  screenMode;
	char const*   title;
};

struct EEApplication {
	EEWindow*	pWindow;
	
	void* comp;
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