#include "pin.H"
#include <iostream>
#include <fstream>
using std::hex;
using std::cerr;
using std::string;
using std::ios;
using std::endl;

/* Names of malloc */
#if defined(TARGET_MAC)
#define MALLOC "_malloc"
#else
#define MALLOC "malloc"
#endif

//Reg Info
static REG RegTinfo;

// The running count of malloc calls
static UINT64 tot_mcount = 0;

//The amount of memeory allocated
static UINT64 tot_memory_alloc = 0;

//Output Trace File
std::ofstream TraceFile;

struct MCOUNT {

    MCOUNT() : mcount(0), mem_alloc(0), temp_alloc(0) {}

    //Holds the instances of Malloc Count for each Thread
    UINT64 mcount;
    //Total Memory allocated
    UINT64 mem_alloc;
    //Temporarily holds the last malloc's memory and adds to the total based on exit status
    UINT64 temp_alloc;
};

//Map to keep track of thread and MCOUTN struct
typedef std::map<THREADID, MCOUNT *> MINFO_MAP;
static MINFO_MAP MemInfo;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "log/malloctrace.out", "specify trace file name");

//Analysis Routine before the call to Malloc
VOID MallocBefore(ADDRINT size, ADDRINT addrinfo)
{
    MCOUNT* m_info = reinterpret_cast<MCOUNT *>(addrinfo);

    //On each invokation, increment the call to Malloc
    m_info->mcount += 1;

    //The input argument to malloc specifies the memory size requrested.
    //The value is not immediatley added to the pool size, we check the return to see if malloc succeded
    m_info->temp_alloc = (UINT64)size;
}

//Analysis routine after Malloc returns
VOID MallocAfter(ADDRINT ret, ADDRINT addrinfo)
{
    MCOUNT* m_info = reinterpret_cast<MCOUNT *>(addrinfo);

    //Check if the return value is greater than 0
    UINT64 ret_int = (UINT64)ret;
    m_info->mem_alloc += (ret_int > 0) ? m_info->temp_alloc : 0;

    //Reset the Temp Alloc variable
    m_info->temp_alloc = 0;
}
   
VOID Image(IMG img, VOID *v)
{
    // Instrument the malloc() function.
    // Find the malloc() function.
    RTN mallocRtn = RTN_FindByName(img, MALLOC);
    if (RTN_Valid(mallocRtn))
    {
        RTN_Open(mallocRtn);
        // Instrument malloc() to print the input argument value - Size of memory allocation
        RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)MallocBefore,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_REG_VALUE, RegTinfo, 
                       IARG_THREAD_ID,
                       IARG_END);
        RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
                       IARG_FUNCRET_EXITPOINT_VALUE, 
                       IARG_REG_VALUE, RegTinfo, 
                       IARG_THREAD_ID,
                       IARG_END);
        RTN_Close(mallocRtn);
    }
}


static VOID OnThreadStart(THREADID tid, CONTEXT *ctxt, INT32, VOID *)
{
    MCOUNT *minfo = new MCOUNT();
    MemInfo.insert(std::make_pair(tid, minfo));
    PIN_SetContextReg(ctxt, RegTinfo, reinterpret_cast<ADDRINT>(minfo));
}

static VOID OnThreadEnd(THREADID tid, const CONTEXT *ctxt, INT32, VOID *)
{
    MINFO_MAP::iterator it = MemInfo.find(tid);
    tot_mcount += it->second->mcount;
    tot_memory_alloc += it->second->mem_alloc;
}

VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    TraceFile.setf(ios::showbase);
    TraceFile << "Malloc Calls : " << tot_mcount << endl;
    TraceFile << "Total Memory Allocated : " << tot_memory_alloc << endl;
    for(MINFO_MAP::iterator iter = MemInfo.begin() ; iter != MemInfo.end(); iter++) {
        TraceFile << "Thread " << iter->first << " Malloc Calls : " << iter->second->mcount <<endl;
        TraceFile << "Thread " << iter->first << " Total Memory Allocated : " <<iter->second->mem_alloc <<endl;
    }
    TraceFile.close();
}
 
INT32 Usage()
{
    cerr << "This tool produces a trace of calls to malloc." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char *argv[])
{
    // Initialize pin & symbol manager
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();
    
    // Write to a file since cout and cerr maybe closed by the application
    TraceFile.open(KnobOutputFile.Value().c_str());
    
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

    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    // Never returns
    PIN_StartProgram();
    
    return 0;
}