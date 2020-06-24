#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;
ofstream OutFile;

// Virtual register we use to point to each thread's TINFO structure.
//
static REG RegTinfo;

// Information about each thread.
//
struct TINFO
{
    TINFO(ADDRINT base) : _stackBase(base), _max(0) {}

    ADDRINT _stackBase;     // Base (highest address) of stack.
    size_t _max;            // Maximum stack usage so far.
};

typedef std::map<THREADID, size_t> THREAD_SIZE;
typedef std::map<THREADID, TINFO *> TINFO_MAP;
static TINFO_MAP ThreadInfos;
static THREAD_SIZE ThreadMaxSize;

VOID OnStackChangeIf(ADDRINT sp, ADDRINT addrInfo)
{
    TINFO *tinfo = reinterpret_cast<TINFO *>(addrInfo);

    // The stack pointer may go above the base slightly.  (For example, the application's dynamic
    // loader does this briefly during start-up.)
    //
    if (sp > tinfo->_stackBase)
        return;

    // Keep track of the maximum stack usage.
    //
    size_t size = tinfo->_stackBase - sp;
    if (size > tinfo->_max){
        tinfo->_max = size;
    }
}

// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Instruction(INS ins, VOID *v)
{
    if (INS_RegWContain(ins, REG_STACK_PTR))
    {
        if (INS_IsSysenter(ins)) return; // no need to instrument system calls

        IPOINT where = IPOINT_AFTER;
        if (!INS_IsValidForIpointAfter(ins))
        {
            if (INS_IsValidForIpointTakenBranch(ins))
            {
                where = IPOINT_TAKEN_BRANCH;
            }
            else
            {
                return;
            }
        }
        INS_InsertCall(ins, where, (AFUNPTR)OnStackChangeIf, IARG_REG_VALUE, REG_STACK_PTR, 
                        IARG_REG_VALUE, RegTinfo, 
                        IARG_THREAD_ID,
                        IARG_END);
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "log/stacktrace.log", "specify output file name");

static VOID OnThreadStart(THREADID tid, CONTEXT *ctxt, INT32, VOID *)
{
    TINFO *tinfo = new TINFO(PIN_GetContextReg(ctxt, REG_STACK_PTR));
    ThreadInfos.insert(std::make_pair(tid, tinfo));
    ThreadMaxSize.insert(std::make_pair(tid, 0));
    PIN_SetContextReg(ctxt, RegTinfo, reinterpret_cast<ADDRINT>(tinfo));
}

static VOID OnThreadEnd(THREADID tid, const CONTEXT *ctxt, INT32, VOID *)
{
    TINFO_MAP::iterator it = ThreadInfos.find(tid);
    ThreadMaxSize[tid] = it->second->_max;
    if (it != ThreadInfos.end())
    {
        delete it->second;
        ThreadInfos.erase(it);
    }
}


// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    for(std::map<THREADID, size_t>::iterator iter = ThreadMaxSize.begin(); 
        iter != ThreadMaxSize.end(); 
        iter++) {
            OutFile<<"Thread ID : "<<iter->first<<" Max Stack Utilization : "<<iter->second<<endl;
    }
    OutFile.close();
}

INT32 Usage()
{
    cerr << "This tool traces the stack of a given executable" << endl;
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

    PIN_AddThreadStartFunction(OnThreadStart, 0);
    PIN_AddThreadFiniFunction(OnThreadEnd, 0);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}