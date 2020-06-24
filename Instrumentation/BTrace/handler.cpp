#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <unistd.h>
#include <linux/net.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "handler.hpp"

void SyscallHandler::handle_call(ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4) {
    //Branch to Different Routines on resolving the system call number
    switch(num) {
        case SYS_open:
            this->open_call(arg0, arg1, arg2);
            break;
        case SYS_read:
            this->read_call(arg0, arg1, arg2);
            break;
        case SYS_write:
            this->write_call(arg0, arg1, arg2);
            break;
        case SYS_close:
            this->close_call(arg0);
            break;
        case SYS_mmap:
        case SYS_mmap2:
            this->mmap_call(arg0, arg1, arg2, arg3, arg4);
            break;
        case SYS_brk:
            this->brk_call(arg0);
            break;
        case SYS_access:
            this->access_call(arg0, arg1);
            break;
        case SYS_fstat:
        case SYS_fstat64:
            this->fstat_call(arg0, arg1);
            break;
        case SYS_stat:
        case SYS_stat64:
            this->lstat_stat_call(arg0, arg1, 0);
            break;
        case SYS_lstat:
        case SYS_lstat64:
            this->lstat_stat_call(arg0, arg1, 1);
            break;
        case SYS_socketcall:
            this->syssocket_check(arg0, arg1);
            break;
        default:
            //Print the Default output
            this->default_handler(num, arg0, arg1, arg2, arg3, arg4);
    }
}

void SyscallHandler::sys_exit(ADDRINT ret) {
    //Retrieve the last Output from the list
    string* sys_op = this->syscall_output.back();

    stringstream ret_val;

    //Genrate the return string
    string dec_str = decstr((INT32)ret);
    if (atoi(dec_str.c_str()) <= 100 && atoi(dec_str.c_str()) > -10)
        ret_val << "returns: " << dec_str << "\n";
    else
        ret_val << "returns: " << hexstr(ret) << "\n";
        
    //Append the string to the last system call output pushed to the list
    sys_op->append(string(ret_val.str()));
}

void SyscallHandler::handle_empty_return() {
    //Returns that are Unkonwn
    for(vector<string*>::iterator iter=this->syscall_output.begin();
            iter < this->syscall_output.end(); iter++) {
                if((*iter)->find("returns")==std::string::npos) {
                    (*iter)->append("returns : UNKONWN\n");
                }
    }
}

void SyscallHandler::print_result() {

    //Handle Empty Returns
    this->handle_empty_return();

    //Writing Output to a Log File
    ofstream out_file;
    out_file.open("log/btrace.log");

    //Output the Result to a File
    for(vector<string*>::iterator iter=this->syscall_output.begin();
            iter < this->syscall_output.end(); iter++) {
                out_file<<*(*iter);
    }
    out_file.close();
}

void SyscallHandler::push_to_console() { 
    //Write using Pipes to the console
    FILE* fp = popen("/bin/cat", "w");

    //Write each string in the buffer
    for(vector<string*>::iterator iter=this->syscall_output.begin();
            iter < this->syscall_output.end(); iter++) {
                fwrite((*iter)->c_str(), sizeof(char), (*iter)->size(), fp);
    }

    //Print a newline char
    fwrite("\n", sizeof(char), 1, fp);

    //Closing the Pipe
    pclose(fp);
}

void SyscallHandler::default_handler(ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4) {
    stringstream sys_op;
    //Ouput as a String stream
    sys_op << decstr(num) << "(" << hexstr(arg0) << "," << hexstr(arg1) << "," << hexstr(arg2) << "," << hexstr(arg3) << "," << hexstr(arg4) << ")";
    //Push output to a buffer
    this->syscall_output.push_back(new string(sys_op.str()));
}

void SyscallHandler::open_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2) {
    //Crating the SYS Open optput
    string* open_call = new string("open(\"");
    //Storing the second ARG :(Flag) as a string
    string flags(hexstr(arg1));

    //Getting the File Path from the address using SafeCopy
    ADDRINT file_ptr;
    PIN_SafeCopy(&file_ptr, &arg0, sizeof(ADDRINT));
    open_call->append((char*)file_ptr);
    open_call->append("\",");

    //Restoring the Flag Value, else writing the value as Hex
    if(flags == "0x80000") {
        open_call->append("O_RDONLY|O_CLOEXEC)");
    } else if (flags == "0x8000") {
        open_call->append("O_RDONLY|O_LARGEFILE)");
    } else if (flags == "0x88000") {
        open_call->append("O_RDONLY|O_LARGEFILE|O_CLOEXEC)");
    } else if (flags == "0x98800") {
        open_call->append("O_RDONLY|O_NONBLOCK|O_LARGEFILE|O_DIRECTORY|O_CLOEXEC");
    } else {
        open_call->append(flags + ")");
    }

    //Adding to Memeory Buffer
    this->syscall_output.push_back(open_call);
}

void SyscallHandler::close_call(ADDRINT arg0) {
    //Creating SYS close output
    string* close_out = new string("close(");

    //Adding the File Descriptor
    close_out->append(decstr(arg0) + ")");

    //Adding to the Memory Buffer
    this->syscall_output.push_back(close_out);
}

void SyscallHandler::write_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2) {
    //Creating SYS write output
    string* write_call = new string("write(");

    //Adding the File Descriptor
    write_call->append(decstr(arg0) + ",\"");

    //Getting the Values of the buffer
    ADDRINT buff_ptr;
    PIN_SafeCopy(&buff_ptr, &arg1, sizeof(ADDRINT));

    //Modify the buffer to remove newlines and only take in 10 characters
    string buffer((char*)buff_ptr);
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    if(buffer.size() >= 10){
        write_call->append(buffer.substr(0,9));
        write_call->append("\"...,");
    }
    else{
        write_call->append(buffer.substr(0, buffer.size()-1));
        write_call->append("\",");
    }

    //Appending the Bytes Written
    write_call->append(decstr(arg2) + ")");

    //Adding to Memeory Buffer
    this->syscall_output.push_back(write_call);
}

void SyscallHandler::read_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2) {
    //Creating SYS write output
    string* read_call = new string("read(");

    //Adding the File Descriptor
    read_call->append(decstr(arg0) + ",\"");

    //Getting the Values of the buffer
    ADDRINT buff_ptr;
    PIN_SafeCopy(&buff_ptr, &arg1, sizeof(ADDRINT));

    //Modify the buffer to remove newlines and only take in 10 characters
    string buffer((char*)buff_ptr);
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    if(buffer.size() >= 10){
        read_call->append(buffer.substr(0,9));
        read_call->append("\"...,");
    }
    else{
        read_call->append(buffer.substr(0, buffer.size()-1));
        read_call->append("\",");
    }

    //Appending the Bytes Written
    read_call->append(decstr(arg2) + ")");

    //Adding to Memeory Buffer
    this->syscall_output.push_back(read_call);
}

void SyscallHandler::mmap_call(ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4) {
    //Generating the Output of MMAP
    string* mmap_out = new string("mmap(");
    //Adding the Address
    mmap_out->append(hexstr(arg0) + ",");
    //Length
    mmap_out->append(decstr(arg1) + ",");
    //Prot flag
    string prot(decstr(arg2));
    if (prot == "0")
        mmap_out->append("PROT_NONE,");
    else if (prot == "1")
        mmap_out->append("PROT_READ,");
    else if (prot == "2")
        mmap_out->append("PROT_WRITE,");
    else if (prot == "3")
        mmap_out->append("PROT_READ|PROT_WRITE,");
    else if (prot == "5")
        mmap_out->append("PROT_READ|PROT_EXEC,");
    else if (prot == "6")
        mmap_out->append("PROT_WRITE|PROT_EXEC,");
    //Flag
    mmap_out->append(decstr(arg3) + ",");
    //File Descriptor
    mmap_out->append(decstr(arg4) + ")");

    //Adding to Memeory Buffer
    this->syscall_output.push_back(mmap_out);
}

void SyscallHandler::brk_call(ADDRINT arg0) {
    //Generating Brk output
    string* brk_out = new string("brk(");

    //Adding the Data Segment address
    brk_out->append(hexstr(arg0) + ")");

    //Adding to the Memory Buffer
    this->syscall_output.push_back(brk_out);
}

void SyscallHandler::access_call(ADDRINT arg0, ADDRINT arg1) {
    //Generating Access Output
    string* access_out = new string("access(\"");

    //Retrieving the File Path
    ADDRINT file_ptr;
    PIN_SafeCopy(&file_ptr, &arg0, sizeof(ADDRINT));
    string file_path((char*)file_ptr);
    access_out->append(file_path + "\",");

    //Adding the Mode
    string flag(decstr(arg1));
    if (flag == "0")
        access_out->append("F_OK)");
    else if(flag == "1")
        access_out->append("X_OK)");
    else if(flag == "2")
        access_out->append("W_OK)");
    else if(flag == "4")
        access_out->append("R_OK)");
    else if(flag == "3")
        access_out->append("X_OK|R_OK)");
    else if(flag == "5")
        access_out->append("X_OK|W_OK)");
    else if(flag == "6")
        access_out->append("R_OK|W_OK)");
    else if(flag == "7")
        access_out->append("X_OK|R_OK|W_OK)");
    else
        access_out->append(flag + ")");

    //Appending to the Buffer
    this->syscall_output.push_back(access_out);
}

void SyscallHandler::fstat_call(ADDRINT arg0, ADDRINT arg1) {
    //Generating Fstat call
    string* fstat_out = new string("fstat(");

    //Adding the FD
    fstat_out->append(decstr(arg0) + ",");

    //Adding the stat buffer addr
    fstat_out->append(hexstr(arg1) + ")");

    //Adding the result to the buffer
    this->syscall_output.push_back(fstat_out);
}

void SyscallHandler::lstat_stat_call(ADDRINT arg0, ADDRINT arg1, int type) {
    //Generate the ouput based on the type
    string* stat_out = new string();
    if (type)
        stat_out->append("lstat(\"");
    else
        stat_out->append("stat(\"");

    //Adding the file Path
    ADDRINT* file_ptr;
    PIN_SafeCopy(&file_ptr, &arg0, sizeof(ADDRINT));
    string file_path((char*)file_ptr);
    stat_out->append(file_path + "\",");

    //Adding the Buffer Addr
    stat_out->append(hexstr(arg1) + ")");

    //Adding the result to the buffer
    this->syscall_output.push_back(stat_out);
}

void SyscallHandler::syssocket_check(ADDRINT arg0, ADDRINT arg1) {
    //Check the type of Socket Call and call the sub-routine
    switch((int)arg0) {
        case SYS_SOCKET:
            this->socket_call(arg1);
            break;
        case SYS_CONNECT:
            this->connect_call(arg1);
            break;
        default:
            this->default_syssocket(arg0, arg1);
    }
}

void SyscallHandler::default_syssocket(ADDRINT arg0, ADDRINT arg1) {
    //Gneric Socket call output
    string* socket_out = new string("socketcall(");

    //Call number
    socket_out->append(decstr(arg0) + ",");

    //Address of buffer
    socket_out->append(hexstr(arg1) + ")");

    //Add to the buffer
    this->syscall_output.push_back(socket_out);
}

void SyscallHandler::socket_call(ADDRINT arg1) {
    //Generate the output for socket
    string* socket_out = new string("socket(");

    //Cast arg1 as memory list
    ADDRINT* socket_args = reinterpret_cast<ADDRINT*>(arg1);

    //Add the Domain
    if ((int)socket_args[0] == AF_INET)
        socket_out->append("AF_INET,");
    else if ((int)socket_args[0] == AF_INET6)
        socket_out->append("AF_INET6,");
    else if ((int)socket_args[0] == AF_LOCAL)
        socket_out->append("AF_LOCAL,");
    else if ((int)socket_args[0] == AF_NETLINK)
        socket_out->append("AF_NETLINK,");
    else
        socket_out->append(decstr(socket_args[0]) + ",");

    //Add the type
    if ((int)socket_args[1] == (SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK))
        socket_out->append("SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK,");
    else if ((int)socket_args[1] == (SOCK_DGRAM|SOCK_NONBLOCK))
        socket_out->append("SOCK_DGRAM|SOCK_NONBLOCK,");
    else if ((int)socket_args[1] == SOCK_RAW)
        socket_out->append("SOCK_RAW,");
    else if ((int)socket_args[1] == SOCK_DGRAM)
        socket_out->append("SOCK_DGRAM,");
    else if ((int)socket_args[1] == SOCK_STREAM)
        socket_out->append("SOCK_STREAM,");
    else
        socket_out->append(decstr(socket_args[1]) + ",");
    
    //Add the Protocol
    socket_out->append(decstr(socket_args[2]) + ")");

    //Add to the buffer
    this->syscall_output.push_back(socket_out);
}

void SyscallHandler::connect_call(ADDRINT arg1) { 
    //Generate the output for socket
    string* connect_out = new string("connect(");

    //Cast arg1 as memory list
    ADDRINT* connect_args = reinterpret_cast<ADDRINT*>(arg1);

    //Add Socket FD
    connect_out->append(decstr(connect_args[0]) + ",");

    //Get Length
    int sock_len = atoi(decstr(connect_args[2]).c_str());

    //Adding the struct
    ADDRINT* sock_addr_ptr;
    PIN_SafeCopy(&sock_addr_ptr, &arg1, sizeof(ADDRINT));
    if (sock_len == 16){
        //Document IP v4 connect call
        struct sockaddr_in* sock_ptr = reinterpret_cast<struct sockaddr_in*>(sock_addr_ptr);
        stringstream out;
        out << "sockaddr_in{" << "Family: AF_INET," << "Port: " << ntohs(sock_ptr->sin_port) << ",";
        out << "IP_Address : "<< inet_ntoa(sock_ptr->sin_addr) << "},";
        connect_out->append(out.str());
    } else if (sock_len == 28) {
        //Document IP v6 connect call
        struct sockaddr_in6* sock_ptr = reinterpret_cast<struct sockaddr_in6*>(sock_addr_ptr);
        stringstream out;
        char ip_addr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(sock_ptr->sin6_addr), ip_addr, INET6_ADDRSTRLEN);
        out << "sockaddr_in{" << "Family: AF_INET6," << "Port: " << ntohs(sock_ptr->sin6_port) << ",";
        out << "IP_Address : "<< ip_addr << "},";
        connect_out->append(out.str());
    } else {
        connect_out->append(hexstr(connect_args[1]) + ",");
    }

    //Appending the Length
    connect_out->append(decstr(connect_args[2]) + ")");
    
    //Add to the buffer
    this->syscall_output.push_back(connect_out);
}
