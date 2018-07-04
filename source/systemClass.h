/////////////////////////////////////////////////////////////////////
// Filename: systemClass.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once



class System
{
public:
  System() {}
  ~System() {}
  System(const System&) = delete;
  System(System&&) = delete;
  System& operator=(const System&) = delete;
  System& operator=(System&&) = delete;
  
  int Run();

private:
  bool InitSystems();
  bool InitContent();
  int GameLoop();
  void Release();
  void ProcessInput();
  void Update();

private:


  bool m_IsRunning{ false };
};