#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

enum file_type_op{File, Network};

void file_open(char*);

void file_read(char*,int);

void file_write(char*,int);

void network_socket(char*);

void network_connect(char*, int, char*, int);

void network_send_msg(char*, int);

void network_send_msg_ip(char*, char*, int);

void network_recv_msg(char*, int);

void network_recv_msg_ip(char*, char*, int);

void memory_map(char*, long, long, int);

void memory_dup(char*);

void initalize_file_info_entry();

int get_file_index(char*);

int get_file_index_addr(long);

int get_file_index_ip(char*);

void initalize_file_list();

void resize_file_list();

void get_file_name(char*, char*);

void print_details(char*);

void print_file_details(char*);

void print_network_details(char*);

void print_memory_details(char*);

#endif