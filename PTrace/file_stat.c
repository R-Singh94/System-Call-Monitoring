#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "file_stat.h"

struct file_info
{
    char file_name[0xFF];
    long addrx;
    enum file_type_op file_op;
    long bytes_read;
    long bytes_written;
    int v4_port;
    int v6_port;
    char ip_v4_address[50];
    char ip_v6_address[100];
    long bytes_sent;
    long bytes_received;
    long bytes_map_read;
    long bytes_map_write;
};

struct file_info *file_info_list;
int file_info_size;
int file_info_current_size;

void print_entry(int index) {
    printf("File Name : %s\n", file_info_list[index].file_name);
    printf("Type : %d\n", file_info_list[index].file_op);
    printf("Addr Info : %ld\n", file_info_list[index].addrx);
}

void file_open(char* file_name) {
    int index;
    index = get_file_index(file_name);
    if (index == -1) {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = File;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
    }
}

void file_read(char* file_name, int bytes_read) {
    int index;
    index = get_file_index(file_name);
    if (index >= 0){
        file_info_list[index].bytes_read += bytes_read;
    } 
    else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = File;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[file_info_current_size-1].bytes_read += bytes_read;
    }
}

void file_write(char* file_name, int bytes_written){
    int index;
    index = get_file_index(file_name);
    if (index >= 0){
        file_info_list[index].bytes_written += bytes_written;
    } 
    else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = File;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[file_info_current_size-1].bytes_written += bytes_written;
    }
}

void network_socket(char* file_name) {
    int index;
    index = get_file_index(file_name);
    if (index == -1){
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = Network;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
    }
}

void network_connect(char* file_name, int port, char* ip_addr, int type){
    int index;
    index = get_file_index(file_name);
    if (index >= 0){
        if (type == 4) {
            file_info_list[index].v4_port = port;
            strcpy(file_info_list[index].ip_v4_address, ip_addr);
        } else if (type ==6){
            file_info_list[index].v6_port = port;
            strcpy(file_info_list[index].ip_v6_address, ip_addr);
        }
    } else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = Network;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        if (type == 4){
            file_info_list[file_info_current_size-1].v4_port = port;
            strcpy(file_info_list[file_info_current_size-1].ip_v4_address, ip_addr);
        } else if(type == 6) {
            file_info_list[file_info_current_size-1].v6_port = port;
            strcpy(file_info_list[file_info_current_size-1].ip_v6_address, ip_addr);
        }
    }
}

void network_send_msg_ip(char* file_name, char* ip_addr, int bytes_sent){
    int index;
    index = get_file_index_ip(ip_addr);
    if (index == -1)
        index = get_file_index(file_name);
    if (index >=0){
        file_info_list[index].bytes_sent += bytes_sent;
    } else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = Network;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[index].bytes_sent += bytes_sent;
    }
}

void network_send_msg(char* file_name, int bytes_sent){
    int index;
    index = get_file_index(file_name);
    if (index >=0){
        file_info_list[index].bytes_sent += bytes_sent;
    } else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = Network;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[index].bytes_sent += bytes_sent;
    }
}

void network_recv_msg(char* file_name, int bytes_received){
    int index;
    index = get_file_index(file_name);
    if (index >=0){
        file_info_list[index].bytes_received += bytes_received;
    } else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = Network;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[index].bytes_received += bytes_received;
    }
}

void network_recv_msg_ip(char* file_name, char* ip_addr, int bytes_received){
    int index;
    index = get_file_index_ip(ip_addr);
    if (index == -1)
        index = get_file_index(file_name);
    if (index >=0){
        file_info_list[index].bytes_received += bytes_received;
    } else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = Network;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[index].bytes_received += bytes_received;
    }
}

void memory_map(char* file_name, long addr, long size, int type){
    int index;
    index = get_file_index(file_name);
    if (index == -1)
        index = get_file_index_addr(addr);
    if (index >= 0){
        if (type == 1) {
            file_info_list[index].bytes_map_read += size;
        } else if (type == 2) {
            file_info_list[index].bytes_map_write += size;
        } else if (type == 3) {
            file_info_list[index].bytes_map_read += size;
            file_info_list[index].bytes_map_write += size;
        }
    } else {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = File;
        if (strlen(file_name) > 0)
            strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
        file_info_list[file_info_current_size-1].addrx = addr;
        if (type == 1) {
            file_info_list[file_info_current_size-1].bytes_map_read += size;
        } else if (type == 2) {
            file_info_list[file_info_current_size-1].bytes_map_write += size;
        } else if (type == 3) {
            file_info_list[file_info_current_size-1].bytes_map_read += size;
            file_info_list[file_info_current_size-1].bytes_map_write += size;
        }
    }
}

void memory_dup(char* file_name) {
    int index;
    index = get_file_index(file_name);
    if (index == -1) {
        initalize_file_info_entry();
        file_info_list[file_info_current_size-1].file_op = File;
        strcpy(file_info_list[file_info_current_size-1].file_name, file_name);
    }
}

void initalize_file_info_entry() {
    if (file_info_current_size >= file_info_size)
        resize_file_list();
    file_info_list[file_info_current_size].file_name[0] = '\0';
    file_info_list[file_info_current_size].bytes_read = 0;
    file_info_list[file_info_current_size].bytes_written = 0;
    file_info_list[file_info_current_size].v4_port = -1;
    file_info_list[file_info_current_size].v6_port = -1;
    file_info_list[file_info_current_size].ip_v4_address[0] = '\0';
    file_info_list[file_info_current_size].ip_v6_address[0] = '\0';
    file_info_list[file_info_current_size].bytes_sent = 0;
    file_info_list[file_info_current_size].bytes_received = 0;
    file_info_current_size += 1;
}

int get_file_index(char* file_name){
    int index = -1;
    for(int i=0; i < file_info_current_size; i++){
        if (strcmp(file_info_list[i].file_name, file_name) == 0)
            index = i;
    }
    return index;
}

int get_file_index_addr(long addr){
    int index = -1;
    for(int i=0; i < file_info_current_size; i++){
        if (file_info_list[i].addrx == addr)
            index = i;
    }
    return index;
}

int get_file_index_ip(char* ip_addr) {
    int index = -1;
    for(int i=0; i < file_info_current_size; i++){
        if (strcmp(file_info_list[i].ip_v4_address, ip_addr) == 0)
            index = i;
    }
    return index;
}

void initalize_file_list() {
    file_info_list = (struct file_info*)malloc(100*sizeof(struct file_info));
    file_info_size = 100;
    file_info_current_size = 0;
}

void resize_file_list() {
    file_info_list = (struct file_info*)realloc(file_info_list, (file_info_size+100)*sizeof(struct file_info));
    file_info_size += 100;
}

void get_file_name(char* time_stamp, char* program) {
    time_t rawtime;
    struct tm* time_info;

    time(&rawtime);
    time_info = localtime(&rawtime);

    sprintf(time_stamp, "%s", program);
    sprintf(time_stamp+strlen(program), "_%d", (1900+time_info->tm_year));
    sprintf(time_stamp+strlen(program)+5, "_%.2d", (time_info->tm_mon+1));
    sprintf(time_stamp+strlen(program)+8, "_%.2d", time_info->tm_mday);
    sprintf(time_stamp+strlen(program)+11, "_%.2d", time_info->tm_hour);
    sprintf(time_stamp+strlen(program)+14, "_%.2d", time_info->tm_min);
    sprintf(time_stamp+strlen(program)+17, "_%.2d", time_info->tm_sec);
    time_stamp[strlen(program)+20] = '\0';
}

void print_details(char* program) {
    char log_file_name[50];
    get_file_name(log_file_name, program);
    print_file_details(log_file_name);
    print_network_details(log_file_name);
    printf("All Log Files Generated\n");
}

bool valid_entry(struct file_info* entry){
    bool ret_val = true;
    if (entry->bytes_read == 0 && entry->bytes_written == 0 
        && entry->bytes_sent == 0 && entry->bytes_received == 0
        && strlen(entry->ip_v4_address) == 0 && strlen(entry->ip_v6_address) == 0)
        ret_val = false;
    return ret_val;
}

void print_network_details(char* file_name) {
    FILE *fp;
    char log_file_name[100];
    log_file_name[0] = '\0';
    strcat(log_file_name, "./logs/network_");
    strcat(log_file_name, file_name);
    strcat(log_file_name, ".csv");
    fp = fopen(log_file_name, "w+");
    fprintf(fp, "File Descriptor,IP V4 Address,Port,IP v6 Address,Port,Bytes Read,Bytes Written,Bytes Sent,Bytes Received\n");
    for(int i=0; i < file_info_current_size; i++){
        if(file_info_list[i].file_op == Network && valid_entry(file_info_list+i)){
            fprintf(fp, "%s,%s,%d,%s,%d,%ld,%ld,%ld,%ld\n", 
                file_info_list[i].file_name, 
                file_info_list[i].ip_v4_address,
                file_info_list[i].v4_port,
                file_info_list[i].ip_v6_address,
                file_info_list[i].v6_port,
                file_info_list[i].bytes_read,
                file_info_list[i].bytes_written,
                file_info_list[i].bytes_sent,
                file_info_list[i].bytes_received);
        }
    }
    fclose(fp);
    printf("Network Logs Generated\n");
}

void print_file_details(char* file_name){
    FILE *fp;
    char log_file_name[100];
    log_file_name[0] = '\0';
    strcat(log_file_name, "./logs/file_");
    strcat(log_file_name, file_name);
    strcat(log_file_name, ".csv");
    fp = fopen(log_file_name, "w+");
    fprintf(fp, "File Path,Memory Address,Bytes Read,Bytes Written,Memory Read,Memory Write\n");
    for(int i=0; i < file_info_current_size; i++){
        if(file_info_list[i].file_op == File){
            unsigned char* temp_addr = (unsigned char*)&file_info_list[i].addrx;
            fprintf(fp, "%s,%ux,%ld,%ld,%ld,%ld\n", 
                file_info_list[i].file_name, 
                temp_addr,
                file_info_list[i].bytes_read,
                file_info_list[i].bytes_written,
                file_info_list[i].bytes_map_read,
                file_info_list[i].bytes_map_write);
        }
    }
    fclose(fp);
    printf("File Logs Generated\n");
}