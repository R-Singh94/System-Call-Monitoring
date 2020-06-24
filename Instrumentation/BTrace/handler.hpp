#pragma once

#include<sys/syscall.h>
#include<string>
#include<vector>

#include "pin.H"

using namespace std;

class SyscallHandler {
    private:
        /**
         * Stores the results of all the systemcalls, which will be outputed to the display
         */
        vector<string*> syscall_output;

        /**
         * Handles the system call Open
         * 
         * @arg : arg* The file path, flag and mode
         * @return : void
         */
        void open_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2);
        /**
         * Handles the system call Write
         * 
         * @arg : arg* The file handler, buffer and bytes written
         * @return : void
         */
        void write_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2);
        /**
         * Handles the system call Read
         * 
         * @arg : arg* The file handler, buffer and bytes written
         * @return : void
         */
        void read_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2);
        /**
         * Handles the system call Read
         * 
         * @arg : arg* The file handler
         * @return : void
         */
        void close_call(ADDRINT arg0);
        /**
         * Handles the system call mmap
         * 
         * @arg: arg0 : Holds the address of the six args to mmap
         * @return : void
         */
        
        void mmap_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4);
        /**
         * Handles the system call Brk
         * 
         * @arg : arg* The address of data segment
         * @return : void
         */
        void brk_call(ADDRINT arg0);
        /**
         * Handles the system call Access
         * 
         * @arg : arg0 Pathname, arg1 Mode
         * @return : void
         */
        void access_call(ADDRINT arg0, ADDRINT arg1);
        /**
         * Handles the system call Fstat
         * 
         * @arg : arg0 FD, arg1 struct stat addr
         * @return : void
         */
        void fstat_call(ADDRINT arg0, ADDRINT arg1);
        /**
         * Handles the system call Lstat and Sstat
         * 
         * @arg : arg0 file path, arg1 struct stat addr
         * @arg : type : if its a stat or lstat call
         * @return : void
         */
        void lstat_stat_call(ADDRINT arg0, ADDRINT arg1, int type);
        /**
         * Handles the system call Lstat and Sstat
         * 
         * @arg : arg0 Type Of Call, arg1 Address to the args
         * @return : void
         */
        void syssocket_check(ADDRINT arg0, ADDRINT arg1);
        /**
         * Handles the system call Lstat and Sstat
         * 
         * @arg : arg0 Type of System socket call arg1 Address to the args
         * @return : void
         */
        void default_syssocket(ADDRINT arg0, ADDRINT arg1);
        /**
         * Handles the system call socket
         * 
         * @arg : arg1 Address to the args
         * @return : void
         */
        void socket_call(ADDRINT arg1);
        /**
         * Handles the system call connect
         * 
         * @arg : arg1 Address to the args
         * @return : void
         */
        void connect_call(ADDRINT arg1);
        /**
         * Prints the Default system call as the output of strace.cpp in Default Examples
         * 
         * @arg : num - SYSCALL number - eax
         * @arg : arg* - Arguments that are stored in ebx, ecx, edx, esi, edi
         * @return : void
         */
        void default_handler(ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4);
        /**
         * Check if the Syscall value has a return
         * If not add an empty value
         * 
         */
        void handle_empty_return();
    public:
        /**
         * Prints the Tracked system calls to the STD-OUTPUT
         * 
         * @arg : void
         * @return : void
         */
        void print_result();
        /**
         * Handles a System call and redirects the call to the subroutine handler
         * 
         * @arg : num - SYSCALL number - eax
         * @arg : arg* - Arguments that are stored in ebx, ecx, edx, esi, edi
         * @return : void
         */
        void handle_call(ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4);
        /**
         * Appends the return value to the previous system call
         * 
         * @arg : return value
         * @return : void
         */
        void sys_exit(ADDRINT ret);
        /**
         * Writes the Output form the log file to STDOUT
         * 
         * @arg : void
         * @return : void
         */
        void push_to_console();
};