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


///////////////////////////////////////////////////////////////////////////////
// RELEASE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
extern void eeReleaseApplication(EEApplication* app);