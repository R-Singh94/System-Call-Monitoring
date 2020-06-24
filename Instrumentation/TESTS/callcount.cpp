#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;
ofstream OutFile;

//Reg Info
static REG RegTinfo;

// The running count of calls is kept here
static UINT64 tot_direct_count = 0;
static UINT64 tot_indirect_count = 0;
static UINT64 tot_return_count = 0;

struct CALLCOUNT {

    CALLCOUNT() : direct_count(0), indirect_count(0), return_count(0) {}

    //Counts all Direct Trasnfer Flow in the program for a Thread
    UINT64 direct_count;

    //Counts all Indirect Trasnfer Flow in the program for a Thread
    UINT64 indirect_count;

    //Counts all RTN in the program for a Thread
    UINT64 return_count;
};

//Map to keep track of thread and CALLCOUTN struct
typedef std::map<THREADID, CALLCOUNT *> CINFO_MAP;
static CINFO_MAP CallInfo;

VOID directCount(ADDRINT addr) {
    //Cast to the call struct
    CALLCOUNT* c_info = reinterpret_cast<CALLCOUNT *>(addr);

    //Increment the direct call count
    c_info->direct_count++;
}

VOID indirectCount(ADDRINT addr) {
    //Cast to the call struct
    CALLCOUNT* c_info = reinterpret_cast<CALLCOUNT *>(addr);

    //Increment the direct call count
    c_info->indirect_count++;
}

VOID returnCount(ADDRINT addr) {
    //Cast to the call struct
    CALLCOUNT* c_info = reinterpret_cast<CALLCOUNT *>(addr);

    //Increment the direct call count
    c_info->return_count++;
}

// Pin calls this function every time a new basic block is encountered
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins = BBL_InsTail(bbl);
        //Direct Control Flow check
        if (INS_IsDirectControlFlow(ins))
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)directCount, 
                            IARG_REG_VALUE, RegTinfo, 
                            IARG_THREAD_ID,
                            IARG_END);
        //Indirect Control Flow check
        else if (INS_IsIndirectControlFlow(ins))
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)indirectCount, 
                            IARG_REG_VALUE, RegTinfo, 
                            IARG_THREAD_ID,
                            IARG_END);
        //Statments that are neither, but maybe RTN statements
        else
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)returnCount, 
                            IARG_REG_VALUE, RegTinfo, 
                            IARG_THREAD_ID,
                            IARG_END);
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "log/callcount.log", "specify output file name");


static VOID OnThreadStart(THREADID tid, CONTEXT *ctxt, INT32, VOID *)
{
    //Create a scratch reg to store the struct address to be used by the thread
    CALLCOUNT *cinfo = new CALLCOUNT();
    CallInfo.insert(std::make_pair(tid, cinfo));
    PIN_SetContextReg(ctxt, RegTinfo, reinterpret_cast<ADDRINT>(cinfo));
}

static VOID OnThreadEnd(THREADID tid, const CONTEXT *ctxt, INT32, VOID *)
{
    //Accumulate the result of each thred
    CINFO_MAP::iterator it = CallInfo.find(tid);
    tot_direct_count += it->second->direct_count;
    tot_indirect_count += it->second->indirect_count;
    tot_return_count += it->second->return_count;
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    //Print result to Log file
    OutFile.setf(ios::showbase);
    OutFile << "Total Direct Transfer Flow : " << tot_direct_count << endl;
    OutFile << "Total Indirect Transfer Flow : " << tot_indirect_count << endl;
    OutFile << "Total Return Statements : " << tot_return_count << endl;
    for(CINFO_MAP::iterator iter = CallInfo.begin(); iter != CallInfo.end(); iter++) {
        OutFile << "Thread " << iter->first << " Direct Call : " << iter->second->direct_count << endl;
        OutFile << "Thread " << iter->first << " Indirect Call : " << iter->second->indirect_count << endl;
        OutFile << "Thread " << iter->first << " Return Call : " << iter->second->return_count << endl;
    }
    OutFile.close();
}

INT32 Usage()
{
    cerr << "This tool counts the number of control flow statements encountered" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    OutFile.open(KnobOutputFile.Value().c_str());

    RegTinfo = PIN_ClaimToolRegister();
    if (!REG_valid(RegTinfo))
    {
        std::cerr << "Cannot allocate a scratch register.\n";
        std::cerr << std::flush;
        return 1;
    }

    //Called Before the start of a Thread
    PIN_AddThreadStartFunction(OnThreadStart, 0);
    
    //Called After a Thread Exits
    PIN_AddThreadFiniFunction(OnThreadEnd, 0);

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}