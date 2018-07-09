/////////////////////////////////////////////////////////////////////
// Filename: eedefs.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

extern "C"
{

#include <stdint.h>
#include <wchar.h>
#include <stdio.h>

#include <glm/glm.hpp>

#include "keycodes.h"

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


#ifdef _DEBUG

#define EEInvariant(val) assert(val);

#else

#define EEInvariant(val)

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

  /////////////
  // HANDLES //
  /////////////
  EE_DEFINE_HANDLE(EEShader);
  EE_DEFINE_HANDLE(EEMesh);
  EE_DEFINE_HANDLE(EEObject);
  EE_DEFINE_HANDLE(EETexture);


  ///////////
  // ENUMS //
  ///////////
  enum EEWindowFlagBits {
    EE_WINDOW_FLAGS_NONE = 0x00,
    EE_WINDOW_FLAGS_CENTERX = 0x01,
    EE_WINDOW_FLAGS_CENTERY = 0x02,
    EE_WINDOW_FLAGS_CENTER = EE_WINDOW_FLAGS_CENTERY | EE_WINDOW_FLAGS_CENTERX
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
    EE_SPLITSCREEN_LEFT = 0x01,
    EE_SPLITSCREEN_RIGHT = 0x02,
    EE_SPLITSCREEN_TOP = 0x04,
    EE_SPLITSCREEN_BOTTOM = 0x08,
    EE_SPLITSCREEN_TOP_LEFT = EE_SPLITSCREEN_TOP | EE_SPLITSCREEN_LEFT,
    EE_SPLITSCREEN_TOP_RIGHT = EE_SPLITSCREEN_TOP | EE_SPLITSCREEN_RIGHT,
    EE_SPLITSCREEN_BOTTOM_LEFT = EE_SPLITSCREEN_BOTTOM | EE_SPLITSCREEN_LEFT,
    EE_SPLITSCREEN_BOTTOM_RIGHT = EE_SPLITSCREEN_BOTTOM | EE_SPLITSCREEN_RIGHT,
    EE_SPLITSCREEN_UNDEFINED  //< Forbidden if splitscreen was enabled in EEGraphicsCreateInfo
  };

  enum EEShaderInputType {
    EE_SHADER_INPUT_TYPE_OBJ_MESH,
    EE_SHADER_INPUT_TYPE_CUSTOM
  };

  enum EEFormat {
    EE_FORMAT_R32G32_SFLOAT,
    EE_FORMAT_R32G32B32_SFLOAT
  };

  enum EEShaderStage {
    EE_SHADER_STAGE_VERTEX,
    EE_SHADER_STAGE_FRAGMENT
  };

  enum EEUniformType {
    EE_UNIFORM_TYPE_SAMPLER,
    EE_UNIFORM_TYPE_BUFFER
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

  /**********
  * SHADER *
  **********/
  struct EEShaderInputDesc {
    uint32_t location;
    EEFormat format;
    uint32_t offset;
  };

  struct EEShaderInput {
    uint32_t                  shaderInputDescCount;
    const EEShaderInputDesc*  pShaderInputDescs;
    uint32_t                  inputStride;
  };

  struct EEPushConstantDesc {
    EEShaderStage shaderStage;
    uint32_t      size;
    void*         value;
  };
  
  struct EEUniformDesc {
    EEUniformType   uniformType;
    EEShaderStage   shaderStage;
    uint32_t        binding;
    EETexture       texture;    //< Must be valid texture handle if uniform type is sampler
    uint32_t        bufferSize; //< Must be valid size if uniform type is buffer
  };

  struct EEShaderCreateInfo {
    const char*               vertexFileName;
    const char*               fragmentFileName;
    EEShaderInputType         shaderInputType;  //< If not EE_SHADER_INPUT_TYPE_CUSTOM the predefined shader input will be used, check EEVertex
    const EEShaderInput*      pShaderInput;
    const EEPushConstantDesc* pPushConstantDesc;
    uint32_t                  uniformCount;
    const EEUniformDesc*      pUniformDescs;
    uint32_t                  amountObjects;  //< The amount of objects that will use this shader
    bool                      wireframe;
    bool                      clockwise;
  };

  struct EEVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uvCoord;
    glm::vec3 normal;
  };



  /****************
  * EEApplication *
  *****************/
  struct EEApplication {
    EEGraphics* graphics;
    EEWindow* window;

    bool KeyHit(EEKey key) const;
    bool KeyPressed(EEKey key) const;
    void MouseMovement(double& dx, double& dy) const;
  };
}