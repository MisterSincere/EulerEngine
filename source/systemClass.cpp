/////////////////////////////////////////////////////////////////////
// Filename: systemClass.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "systemClass.h"

#include <stdio.h>


int System::Run()
{
  if (!InitSystems())
    return -1;

  if (!InitContent())
    return -2;

  return GameLoop();
}

bool System::InitSystems()
{
  EEWindowCreateInfo windowCInfo;
  windowCInfo.flags = 0;
  windowCInfo.clientSize = { 1024, 768 };
  windowCInfo.position = { 870, 300 };
  windowCInfo.screenMode = EE_SCREEN_MODE_WINDOWED;
  windowCInfo.title = "EulerEngineTest";
  windowCInfo.icon = nullptr;
  windowCInfo.mouseDisabled = false;

  EEGraphicsCreateInfo gfx;
  gfx.splitscreen = EE_SPLITSCREEN_MODE_NONE;

  eeCreateApplication(m_application, &windowCInfo, &gfx);

  return true;
}

bool System::InitContent()
{

  //eeFinishCreation(m_application);

  return true;
}

int System::GameLoop()
{
  m_IsRunning = true;

  while (m_IsRunning && !eePollMessages(m_application))
  {
    ProcessInput();

    Update();
  }

  Release();

  return 0;
}

void System::Release()
{

}

void System::ProcessInput()
{
  if (m_application.KeyHit(EE_KEY_ESCAPE))
  {
    m_IsRunning = false;
  }
}

void System::Update()
{

}