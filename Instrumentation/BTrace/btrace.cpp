/**
 * @Ref : Content referred from PIN-Manual Example strace.cpp
 *
 */

/*
 *  Program to trace system calls
 */

#include <stdio.h>

#include "handler.hpp"
#include "pin.H"

//Custom class that handles generating string args similar to strace
static SyscallHandler syscall_handle;

static bool isSyscallEnc = false;

// Print syscall number and arguments
VOID SysBefore(ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4)
{
    //Call routine to get Agrs before call to system routine
    isSyscallEnc = true;
    syscall_handle.handle_call(num, arg0, arg1, arg2, arg3, arg4);
}

// Print the return value of the system call
VOID SysAfter(ADDRINT ret)
{
    //Call Routine to handle exit from system call
    isSyscallEnc = false;
    syscall_handle.sys_exit(ret);
}

// Is called for every instruction and instruments syscalls
VOID Trace(TRACE trace, VOID *v)
{
    for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        if (isSyscallEnc) {
            BBL_InsertCall(bbl, IPOINT_BEFORE, AFUNPTR(SysAfter),
                                IARG_REG_VALUE, REG_EAX,
                                IARG_END);
        }
        for(INS ins=BBL_InsHead(bbl); INS_Valid(ins); ins=INS_Next(ins)){
            if (INS_IsSyscall(ins))
            {
                // Arguments and syscall number is only available before
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SysBefore),
                            IARG_REG_VALUE, REG_EAX, IARG_REG_VALUE, REG_EBX,
                            IARG_REG_VALUE, REG_ECX, IARG_REG_VALUE, REG_EDX,
                            IARG_REG_VALUE, REG_ESI, IARG_REG_VALUE, REG_EDI,
                            IARG_END);
            }
        }
    }
}


VOID Fini(INT32 code, VOID *v)
{
    //Prints to a Log File
    syscall_handle.print_result();
    //Prints to the console
    syscall_handle.push_to_console();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints a log of system calls"
                + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    //Instrument Each Block
    TRACE_AddInstrumentFunction(Trace, 0);

    //On Exit
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
