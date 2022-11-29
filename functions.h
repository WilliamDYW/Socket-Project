#ifndef FUNC_NETW
#define FUNC_NETW

#include <stdio.h>
#define L3D "874"
#define NODE "127.0.0.1"
#define MAXDATASIZE 8192

int open_socket(char *port_num, int tcp, int self_ip, int server);
int auth_course(char *str, char *code);
void find_course(char *code, FILE *fp, char *subject);
extern char *course_category_list[];
extern int course_category_size;

#endif