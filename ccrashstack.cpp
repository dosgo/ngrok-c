#include "ccrashstack.h"
#include <tlhelp32.h>
#include <stdio.h>

//  ccrashstack.cpp
//
//
//  Created by Lan huaiyu on 9/27/13.
//  Copyright 2013 CCrashStack. All rights reserved.
//
using namespace std;

string wstring2string(wstring wstr)  
{  
    string result;  
    //获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);  
    char* buffer = new char[len + 1];  
    //宽字节编码转换成多字节编码  
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);  
    buffer[len] = '\0';  
    //删除缓冲区并返回值  
    result.append(buffer);  
    delete[] buffer;  
    return result;  
}  

CCrashStack::CCrashStack(PEXCEPTION_POINTERS pException)
{
    m_pException = pException;
}


string CCrashStack::GetModuleByRetAddr(PBYTE Ret_Addr, PBYTE & Module_Addr)
{
    MODULEENTRY32	M = {sizeof(M)};
    HANDLE	hSnapshot;

    wchar_t Module_Name[MAX_PATH] = {0};

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

    if ((hSnapshot != INVALID_HANDLE_VALUE) &&
            Module32First(hSnapshot, &M))
    {
            do
            {
                    if (DWORD(Ret_Addr - M.modBaseAddr) < M.modBaseSize)
                    {
                            lstrcpyn(Module_Name, M.szExePath, MAX_PATH);
                            Module_Addr = M.modBaseAddr;
                            break;
                    }
            } while (Module32Next(hSnapshot, &M));
    }

    CloseHandle(hSnapshot);

    string sRet =wstring2string(Module_Name);
    return sRet;
}

string CCrashStack::GetCallStack(PEXCEPTION_POINTERS pException)
{
        PBYTE	Module_Addr_1;
        char bufer[256]={0};
        string sRet;

        typedef struct STACK
        {
                STACK *	Ebp;
                PBYTE	Ret_Addr;
                DWORD	Param[0];
        } STACK, * PSTACK;

        STACK	Stack = {0, 0};
        PSTACK	Ebp;

        if (pException)		//fake frame for exception address
        {
                Stack.Ebp = (PSTACK)pException->ContextRecord->Ebp;
                Stack.Ret_Addr = (PBYTE)pException->ExceptionRecord->ExceptionAddress;
                Ebp = &Stack;
        }
        else
        {
                Ebp = (PSTACK)&pException - 1;	//frame addr of Get_Call_Stack()

                // Skip frame of Get_Call_Stack().
                if (!IsBadReadPtr(Ebp, sizeof(PSTACK)))
                        Ebp = Ebp->Ebp;		//caller ebp
        }

        // Break trace on wrong stack frame.
        for (; !IsBadReadPtr(Ebp, sizeof(PSTACK)) && !IsBadCodePtr(FARPROC(Ebp->Ret_Addr));
                 Ebp = Ebp->Ebp)
        {
                // If module with Ebp->Ret_Addr found.
                memset(bufer,0, sizeof(0));
                sprintf(bufer, "\n%08X  ", (unsigned int)Ebp->Ret_Addr);
                sRet.append(bufer);

                QString moduleName = this->GetModuleByRetAddr(Ebp->Ret_Addr, Module_Addr_1) ;
                if (moduleName.length() > 0)
                {
                    sRet.append(moduleName);
                }
        }

        return sRet;
} //Get_Call_Stack

string CCrashStack::GetVersionStr()
{
        OSVERSIONINFOEX	V = {sizeof(OSVERSIONINFOEX)};	//EX for NT 5.0 and later

        if (!GetVersionEx((POSVERSIONINFO)&V))
        {
                ZeroMemory(&V, sizeof(V));
                V.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                GetVersionEx((POSVERSIONINFO)&V);
        }

        if (V.dwPlatformId != VER_PLATFORM_WIN32_NT)
                V.dwBuildNumber = LOWORD(V.dwBuildNumber);	//for 9x HIWORD(dwBuildNumber) = 0x04xx

        string sRet = string("Windows:  %1.%2.%3, SP %4.%5, Product Type %6\n")
                .arg(V.dwMajorVersion).arg(V.dwMinorVersion).arg(V.dwBuildNumber)
                .arg(V.wServicePackMajor).arg(V.wServicePackMinor).arg(V.wProductType);
        return sRet;
}

string CCrashStack::GetExceptionInfo()
{
        WCHAR		Module_Name[MAX_PATH];
        PBYTE		Module_Addr;

        string sRet;
        char buffer[512]={0};

        string sTmp = GetVersionStr();
        sRet.append(sTmp);
        sRet.append("Process:  ");

        GetModuleFileName(NULL, Module_Name, MAX_PATH);
        sRet=sRet+wstring2string(Module_Name)+"\n";

        // If exception occurred.
        if (m_pException)
        {
                EXCEPTION_RECORD &	E = *m_pException->ExceptionRecord;
                CONTEXT &			C = *m_pException->ContextRecord;

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "Exception Addr:  %08X  ", (int)E.ExceptionAddress);
                sRet.append(buffer);
                // If module with E.ExceptionAddress found - save its path and date.
                string module = GetModuleByRetAddr((PBYTE)E.ExceptionAddress, Module_Addr);
                if (module.length() > 0)
                {
                    sRet.append(" Module: ");

                    sRet.append(module);
                }

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "\nException Code:  %08X\n", (int)E.ExceptionCode);
                sRet.append(buffer);

                if (E.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
                {
                        // Access violation type - Write/Read.
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer,"%s Address:  %08X\n",
                            (E.ExceptionInformation[0]) ? "Write" : "Read", (int)E.ExceptionInformation[1]);
                    sRet.append(buffer);
                }


                sRet.append("Instruction: ");
                for (int i = 0; i < 16; i++)
                {
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, " %02X",  PBYTE(E.ExceptionAddress)[i]);
                    sRet.append(buffer);
                }

                sRet.append("\nRegisters: ");

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "\nEAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X",  (unsigned int)C.Eax,(unsigned int) C.Ebx, (unsigned int)C.Ecx, (unsigned int)C.Edx);
                sRet.append(buffer);

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "\nESI: %08X  EDI: %08X  ESP: %08X  EBP: %08X", (unsigned int)C.Esi, (unsigned int)C.Edi, (unsigned int)C.Esp, (unsigned int)C.Ebp);
                sRet.append(buffer);

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "\nEIP: %08X  EFlags: %08X", (unsigned int)C.Eip,(unsigned int) C.EFlags);
                sRet.append(buffer);

        } //if (pException)

        sRet.append("\nCall Stack:");
        string sCallstack = this->GetCallStack(m_pException);
        sRet.append(sCallstack);

        return sRet;
}

