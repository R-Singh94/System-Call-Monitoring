#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/socket.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "file_stat.h"

const int long_size = sizeof(long);
int MAXLENGTH = 0xFFF;

#define err_abort(x) do { \
      if (!(x)) {\
         fprintf(stderr, "Fatal error: %s:%d: ", __FILE__, __LINE__);   \
         perror(""); \
         exit(1);\
      }\
   } while (0)

void set_str(char* file_str, int size) {
    for(int i=0; i<size; i++){
        file_str[i] = '\0';
    }
}

void get_file_path(pid_t child_pid, int file_desc, char* file_name) {
    char proclnk[0xFFF];
    ssize_t tot_len;

    sprintf(proclnk, "/proc/%d/fd/%d", child_pid, file_desc);
    tot_len = readlink(proclnk, file_name, MAXLENGTH);
    if (tot_len < 0)
        file_name[0] = '\0';
    else
        file_name[tot_len] = '\0';
}

void get_data(pid_t child, long addr, long *ret_data, int len) {
    int i, j;
    long data;
    i = 0;
    j = len / long_size;
    while(i < j) {
        data = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
        *(ret_data+i) = data;
        i++;
    }
    j = len % long_size;
    if(j != 0) {
        data = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
        *(ret_data+i) = data;
    }
}

int handle_connect_sys(pid_t child_pid, long struct_addr, long struct_size, int *port, char* ip_addr) {
    int type = 0;
    if (struct_size == 16) {
        long struct_data[4];
        struct sockaddr_in* ip_addr_struct;
        get_data(child_pid, struct_addr, struct_data, sizeof(struct sockaddr_in));
        ip_addr_struct = (struct sockaddr_in*)struct_data;
        if (ip_addr_struct->sin_family == AF_INET){
            *port = ntohs(ip_addr_struct->sin_port);
            set_str(ip_addr, 50);
            inet_ntop(AF_INET, &(ip_addr_struct->sin_addr), ip_addr, INET_ADDRSTRLEN);
            type = 4;
        }
    } else if (struct_size == 28) {
        long struct_data[7];
        struct sockaddr_in6* ip_addr_struct;
        get_data(child_pid, struct_addr, struct_data, sizeof(struct sockaddr_in6));
        ip_addr_struct = (struct sockaddr_in6*)struct_data;
        if(ip_addr_struct->sin6_family == AF_INET6) {
            *port = ntohs(ip_addr_struct->sin6_port);
            set_str(ip_addr, 50);
            inet_ntop(AF_INET6, &(ip_addr_struct->sin6_addr), ip_addr, INET6_ADDRSTRLEN);
            type = 6;
        }
    }
    return type;
}

void handle_send_req(pid_t child_pid, long struct_addr, int *port, char* ip_addr) {
    long struct_data[14];
    struct msghdr* msg;
    struct sockaddr_in* ip_addr_struct;
    get_data(child_pid, struct_addr, struct_data, sizeof(struct msghdr));
    msg = (struct msghdr*)struct_data;
    get_data(child_pid, (long)msg->msg_name, struct_data, sizeof(struct sockaddr_in));
    ip_addr_struct = (struct sockaddr_in*)struct_data;
    if (ip_addr_struct->sin_family == AF_INET){
        *port = ntohs(ip_addr_struct->sin_port);
        set_str(ip_addr, 50);
        inet_ntop(AF_INET, &(ip_addr_struct->sin_addr), ip_addr, INET_ADDRSTRLEN);
    }
}


void resolve_socketcall(pid_t child_pid, struct user_regs_struct* regs){
    char file_name[MAXLENGTH];
    switch(regs->ebx){
        case SYS_SOCKET:
            if (regs->eax >= 0){
                set_str(file_name, MAXLENGTH);
                get_file_path(child_pid, regs->eax, file_name);
                network_socket(file_name);
            }
            break;
        case SYS_CONNECT:
            if (regs->eax >= 0){
                long file_desc;
                long struct_addr;
                long struct_size;
                int port, type;
                char ip_address[50];
                get_data(child_pid, regs->ecx, &file_desc, sizeof(long));
                set_str(file_name, MAXLENGTH);
                get_file_path(child_pid, (int)file_desc, file_name);
                get_data(child_pid, regs->ecx+4, &struct_addr, sizeof(long));
                get_data(child_pid, regs->ecx+8, &struct_size, sizeof(long));
                type = handle_connect_sys(child_pid, struct_addr, struct_size, &port, ip_address);
                if (type > 0)
                    network_connect(file_name, port, ip_address, type);
            }
            break;
        case SYS_RECVFROM:
        case SYS_RECV:
            if (regs->eax >= 0){
                long file_desc;
                get_data(child_pid, regs->ecx, &file_desc, sizeof(long));
                set_str(file_name, MAXLENGTH);
                get_file_path(child_pid, (int)file_desc, file_name);
                network_recv_msg(file_name, regs->eax);
            }
            break;
        case SYS_RECVMSG:
            if (regs->eax >= 0) {
                long file_desc;
                long struct_addr;
                int port;
                char ip_address[50];
                get_data(child_pid, regs->ecx, &file_desc, sizeof(long));
                set_str(file_name, MAXLENGTH);
                get_file_path(child_pid, (int)file_desc, file_name);
                get_data(child_pid, regs->ecx+4, &struct_addr, sizeof(long));
                handle_send_req(child_pid, struct_addr, &port, ip_address);
                network_recv_msg_ip(file_name, ip_address, regs->eax);
            }
            break;
        case SYS_SENDMMSG:
        case SYS_SENDMSG:
            if (regs->eax >= 0) {
                long file_desc;
                long struct_addr;
                int port;
                char ip_address[50];
                get_data(child_pid, regs->ecx, &file_desc, sizeof(long));
                set_str(file_name, MAXLENGTH);
                get_file_path(child_pid, (int)file_desc, file_name);
                get_data(child_pid, regs->ecx+4, &struct_addr, sizeof(long));
                handle_send_req(child_pid, struct_addr, &port, ip_address);
                network_send_msg_ip(file_name, ip_address, regs->eax);
            }
            break;
        case SYS_SEND:
        case SYS_SENDTO:
            if (regs->eax >= 0) {
                long file_desc;
                get_data(child_pid, regs->ecx, &file_desc, sizeof(long));
                set_str(file_name, MAXLENGTH);
                get_file_path(child_pid, (int)file_desc, file_name);
                network_send_msg(file_name, regs->eax);
            }
            break;
        default:
            break;
    }
}

void resolve_mmap(pid_t child_pid, struct user_regs_struct* regs) {
    char file_name[MAXLENGTH];
    int type_info;

    set_str(file_name, MAXLENGTH);
    if (regs->edi >= 0)
        get_file_path(child_pid, regs->edi, file_name);
    if((int)regs->edx == 1)
        type_info = 1;
    else if((int)regs->edx == 2)
        type_info = 2;
    else if((int)regs->edx == 3)
        type_info = 3;
    else if((int)regs->edx == 5)
        type_info = 1;
    else if((int)regs->edx == 6)
        type_info = 2;
    memory_map(file_name, regs->eax, regs->ecx, type_info);
}

int main(int argc, char** argv) {
    int status;
    long eax;
    struct user_regs_struct regs;
    int insyscall = 0;
    pid_t child_pid;
    char file_name[MAXLENGTH];

    if (argc < 2) {
        printf("Command used with less number of arguments:\n");
        printf("<USAGE>:\n");
        printf("./command_monitor <COMMAND> <optional>[<ARGS>...]\n");
        exit(1);
    }

    initalize_file_list();

    child_pid = fork();
    if (child_pid == -1){
        fprintf( stderr, "Could not Fork a Child Process\n");
        exit(1);
    }
    else if (child_pid == 0)
    {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        err_abort(execvp(argv[1], argv+1) >= 0);
    }
    else {
        while (1)
        {
            waitpid(child_pid, &status, 0);
            if(WIFEXITED(status))
                break;
            ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
            if (regs.orig_eax == SYS_socketcall)
            {
                if(insyscall == 0) {
                    insyscall = 1;
                }
                else {
                    resolve_socketcall(child_pid, &regs);
                    insyscall = 0;
                }
            }
            else if (regs.orig_eax == SYS_open){
                if(insyscall == 0) {
                    insyscall = 1;
                }
                else {
                    eax = ptrace(PTRACE_PEEKUSER, child_pid, 4 * EAX, NULL);
                    set_str(file_name, MAXLENGTH);
                    get_file_path(child_pid, eax, file_name);
                    if(eax >= 0)
                        file_open(file_name);
                    insyscall = 0;
                }
            }
            else if (regs.orig_eax == SYS_read){
                if(insyscall == 0) {
                    insyscall = 1;
                    set_str(file_name, MAXLENGTH);
                    get_file_path(child_pid, regs.ebx, file_name);
                }
                else {
                    eax = ptrace(PTRACE_PEEKUSER, child_pid, 4 * EAX, NULL);
                    if(eax >= 0)
                        file_read(file_name, eax);
                    insyscall = 0;
                }
            }
            else if (regs.orig_eax == SYS_write){
                if(insyscall == 0) {
                    insyscall = 1;
                    set_str(file_name, MAXLENGTH);
                    get_file_path(child_pid, regs.ebx, file_name);
                }
                else {
                    eax = ptrace(PTRACE_PEEKUSER, child_pid, 4 * EAX, NULL);
                    if(eax >= 0)
                        file_write(file_name, eax);
                    insyscall = 0;
                }
            } else if (regs.orig_eax == SYS_mmap || regs.orig_eax == SYS_mmap2) {
                if(insyscall == 0) {
                    insyscall = 1;
                }
                else {
                    resolve_mmap(child_pid, &regs);
                    insyscall = 0;
                }
            } else if (regs.orig_eax == SYS_dup2 || regs.orig_eax == SYS_dup) {
                if(insyscall == 0) {
                    insyscall = 1;
                }
                else {
                    set_str(file_name, MAXLENGTH);
                    get_file_path(child_pid, regs.eax, file_name);
                    memory_dup(file_name);
                    insyscall = 0;
                }
            }
            ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);    
        }
        print_details(argv[1]);
    }
    return 0;
}