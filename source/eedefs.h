/////////////////////////////////////////////////////////////////////
// Filename: eedefs.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>
#include <wchar.h>
#include <stdio.h>

//////////////////////
// DEFINES / MACROS //
//////////////////////
#define EE_TRUE   1
#define EE_FALSE  0

#define EE_DEFINE_HANDLE(n) typedef uint32_t* n;

#ifdef UNICODE

typedef const wchar_t* EEcstr;
typedef wchar_t* EEstr;

#define EESTR(val) L##val
#define EEPrint(val, ...) wprintf_s(L##val, __VA_ARGS__);

#else

typedef const char* EEcstr;
typedef char* EEstr;

#define EESTR(val) val
#define EEPRINT(val, ...) printf_s(val, __VA_ARGS__);

#endif

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
struct GLFWwindow;

//////////////
// TYPEDEFS //
//////////////
typedef uint32_t EEFlags;
typedef uint32_t EEBool32;


///////////
// ENUMS //
///////////
enum EEWindowFlagBits {
  EE_WINDOW_FLAGS_NONE      = 0x00,
  EE_WINDOW_FLAGS_CENTERX   = 0x01,
  EE_WINDOW_FLAGS_CENTERY   = 0x02,
  EE_WINDOW_FLAGS_CENTER    = EE_WINDOW_FLAGS_CENTERY | EE_WINDOW_FLAGS_CENTERX
};
typedef EEFlags EEWindowFlags;

enum EEScreenMode {
  EE_SCREEN_MODE_WINDOWED,
  EE_SCREEN_MODE_FULLSCREEN,
  EE_SCREEN_MODE_FAKE_FULLSCREEN,
  EE_SCREEN_MODE_MAXIMIZED
};

enum EESplitscreenMode {
  EE_SPLITSCREEN_MODE_NONE = 0x00,
  EE_SPLITSCREEN_MODE_VERTICAL = 0x01,
  EE_SPLITSCREEN_MODE_HORIZONTAL = 0x02,
  EE_SPLITSCREEN_MODE_RASTER = EE_SPLITSCREEN_MODE_HORIZONTAL | EE_SPLITSCREEN_MODE_VERTICAL
};

enum EESplitscreen {
  EE_SPLITSCREEN_LEFT     = 0x01,
  EE_SPLITSCREEN_RIGHT    = 0x02,
  EE_SPLITSCREEN_TOP      = 0x04,
  EE_SPLITSCREEN_BOTTOM   = 0x08,
  EE_SPLITSCREEN_TOP_LEFT       = EE_SPLITSCREEN_TOP | EE_SPLITSCREEN_LEFT,
  EE_SPLITSCREEN_TOP_RIGHT      = EE_SPLITSCREEN_TOP | EE_SPLITSCREEN_RIGHT,
  EE_SPLITSCREEN_BOTTOM_LEFT    = EE_SPLITSCREEN_BOTTOM | EE_SPLITSCREEN_LEFT,
  EE_SPLITSCREEN_BOTTOM_RIGHT   = EE_SPLITSCREEN_BOTTOM | EE_SPLITSCREEN_RIGHT,
  EE_SPLITSCREEN_UNDEFINED  //< Forbidden if splitscreen was enabled in EEGraphicsCreateInfo
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

struct EEGraphics {
  void* comp;
};

struct EEWindow {
  EERect32U     clientSize;
  EEPoint32     position;
  GLFWwindow*   winHandle;
  EEScreenMode  screenMode;
  const char*   title;
};

/***************
* CREATE INFOS *
****************/
struct EEGraphicsCreateInfo {
  EESplitscreenMode splitscreen;
};

struct EEWindowCreateInfo {
  EEWindowFlags flags;
  EERect32U     clientSize;
  EEPoint32     position;
  EEScreenMode  screenMode;
  const char*   title;
  const char*   icon;
  bool          mouseDisabled;
};


/****************
* EEApplication *
*****************/
struct EEApplication {
  EEGraphics* graphics;
  EEWindow* window;
};