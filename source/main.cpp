#include "systemClass.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <iostream>


int main(int argc, char** argv)
{
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  //_CrtSetBreakAlloc(273);

  System* pSys = new System;

  int rv = pSys->Run();
  if (rv) {
    printf_s("System returned with error %d\n", rv);
    system("PAUSE");
  }

  delete pSys;

  return 0;
}