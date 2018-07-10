/////////////////////////////////////////////////////////////////////
// Filename: eulerengine.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once


#include "eedefs.h"



///////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
/**
  * Creates the main application handle you will need for all further operations
  *
  * @param appOut             The application structure that is created
  * @param windowCreateInfo   Pointer to the structure defining the window to create
  * @param graphicsCreateInfo Pointer to the structure defining the graphics options
  *
  * @return Is true if everything went right
  **/
extern bool eeCreateApplication(
  EEApplication& appOut,
  const EEWindowCreateInfo* windowCreateInfo,
  const EEGraphicsCreateInfo* graphicsCreateInfo);

/**
 * Creates a TEXTURE from the file passed in.
 * @param app         The application struct
 * @param file        The destination of the texture file to read in
 * @return            The handle to the texture, that can be set as a resource for shader to use
 **/
extern EETexture eeiCreateTexture(
  EEApplication&  app,
  const char*     file);

/**
 * Creates a SHADER with the settings specified in the EEShaderCreateInfo. Does only use compiled shader in Spir-V.
 *
 * @param app         The application struct
 * @param shaderCInfo The struct that described the details of the shader
 *
 * @return            The handle to the mesh, that can be used to create an object using with this shader
 **/
extern EEShader eeiCreateShader(
  EEApplication&            app,
  const EEShaderCreateInfo& shaderCInfo);

/**
 * Creates an OBJECT that will be drawed everytime eeDrawFrame is called. The object uses the mesh passed in and
 * renders it with the shader passed in. The splitscreen option can be ignored if splitscreen wasn't enabled.
 *
 * @param app         The application struct
 * @param shader      The shader that should be used to render
 * @param mesh        The mesh that should be rendered
 * @param splitscreen The side of the splitscreen the object will be drawn on
 *
 * @return Handle to the created object
 **/
extern EEObject eeiCreateObject(
  EEApplication&  app,
  EEShader        shader,
  EEMesh          mesh,
  EESplitscreen   splitscreen = EE_SPLITSCREEN_UNDEFINED);

/**
 * Finishes up application creation. Every eei function called before this method will now be recorded and hence be rendered.
 * @param app         The application created with eeCreateApplication
 **/
extern void eeFinishCreation(const EEApplication& app);


///////////////////////////////////////////////////////////////////////////////
// UPDATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
/**
 * Polls the messages for the window. Things like key inputs etc.
 *
 * @param application   The application structure you wanna poll the events from
 *
 * @return Is true if an exit message was send
 **/
extern bool eePollMessages(const EEApplication& appliction);

/**
 * Update the uniform buffer of the specified object to the data passed in (pretty simple isn't it ;)
 * @param app     The application detail struct
 * @param object  The object of which you wanna update the uniform buffer
 * @param data    The new data that will be set to the objects uniform buffer
 * @param binding Optional, if you have multiple uniform buffers pass in its binding (0 will use just the first ubo)
 **/
extern void eeUpdateUniformBuffer(
  const EEApplication&  app,
  EEObject              object,
  void*                 data,
  int                   binding = 0);

/**
* Finishes up the frame and draws everything. If you wanna see something, call this method.
* @param app     The application you wanna draw the frame to
**/
extern void eeDrawFrame(const EEApplication& app);


///////////////////////////////////////////////////////////////////////////////
// RELEASE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
extern void eeReleaseApplication(EEApplication* app);