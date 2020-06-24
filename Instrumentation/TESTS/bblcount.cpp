#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;
ofstream OutFile;
// The running count of instructions is kept here
// make it static to help the compiler optimize docount

static REG RegTinfo;

static UINT64 total_count = 0;

struct COUNTINFO {

    COUNTINFO() : count(0) {}

    //Holds the BBL Count of Each Thread
    UINT64 count;
};

typedef std::map<THREADID, COUNTINFO *> CINFO_MAP;
static CINFO_MAP CountInfo;

// This function is called before every block
VOID docount(ADDRINT addrInfo) { 
    //Interpret the scratch reg as the CountInfo Struct
    COUNTINFO* c_info = reinterpret_cast<COUNTINFO *>(addrInfo);

    //Increment the count
    c_info->count++;
}

// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, 
                        IARG_REG_VALUE, RegTinfo, 
                        IARG_THREAD_ID,
                        IARG_END);
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "log/bblcount.log", "specify output file name");

static VOID OnThreadStart(THREADID tid, CONTEXT *ctxt, INT32, VOID *)
{
    COUNTINFO *cinfo = new COUNTINFO();
    CountInfo.insert(std::make_pair(tid, cinfo));
    PIN_SetContextReg(ctxt, RegTinfo, reinterpret_cast<ADDRINT>(cinfo));
}

static VOID OnThreadEnd(THREADID tid, const CONTEXT *ctxt, INT32, VOID *)
{
    CINFO_MAP::iterator it = CountInfo.find(tid);
    total_count += it->second->count;
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    for(CINFO_MAP::iterator iter = CountInfo.begin(); iter != CountInfo.end(); iter++){
        OutFile << "Thread " << iter->first << " Count : " << iter->second->count << endl;
    }
    OutFile << "Total Count " << total_count << endl;
    OutFile.close();
}

INT32 Usage()
{
    cerr << "This tool counts the number of BBL encoutered by Pin" << endl;
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