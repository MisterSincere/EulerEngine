/////////////////////////////////////////////////////////////////////
// Filename: systemClass.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "systemClass.h"


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

  return true;
}

bool System::InitContent()
{

  return true;
}

int System::GameLoop()
{
  m_IsRunning = true;

  while (m_IsRunning)
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

}

void System::Update()
{

}