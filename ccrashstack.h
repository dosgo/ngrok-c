#ifndef CCRASHSTACK_H
#define CCRASHSTACK_H

#include <windows.h>
#include <string>

//
//  ccrashstack.h
//
//
//  Created by Lan huaiyu on 9/27/13.
//  Copyright 2013 CCrashStack. All rights reserved.
//
using namespace std;

class CCrashStack
{
private:
    PEXCEPTION_POINTERS m_pException;

private:
    string GetModuleByRetAddr(PBYTE Ret_Addr, PBYTE & Module_Addr);
    string GetCallStack(PEXCEPTION_POINTERS pException);
    string GetVersionStr();


public:
    CCrashStack(PEXCEPTION_POINTERS pException);

    string GetExceptionInfo();
};

#endif // CCRASHSTACK_H
