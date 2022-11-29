#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "functions.h"

char *course_category_list[] = {"Credit", "Professor", "Days", "CourseName"};
int course_category_size = sizeof(course_category_list)/sizeof(course_category_list[0]);

/*
int open_socket(): This part of code is mainly copied from Beej's Guide to Network Programming,
                   and is slightly modified from client.c, server.c, listener.c and talker.c.
                   port_num is the string of designated port number;
                   tcp will be 1 if TCP socket is created, 0 if UDP;
                   self_ip will be 1 if the socket is using its own ip, otherwise 0;
                   server will be 1 if the socket is a server, 0 if client.
*/
int open_socket(char *port_num, int tcp, int self_ip, int server){
    struct addrinfo hints, *servinfo, *temp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = tcp ? SOCK_STREAM : SOCK_DGRAM;
    if(self_ip)hints.ai_flags = AI_PASSIVE;

    int status;
    if((status = getaddrinfo(self_ip ? NULL : NODE, port_num, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    int socket_fd, yes = 1;
    for(temp = servinfo; temp != NULL; temp = temp->ai_next){        
        if ((socket_fd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1){
            perror("socket");
            continue;
        }
        if(server){
            if(tcp)if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                perror("setsockopt");
                return -2;
            }
            if (bind(socket_fd, temp->ai_addr, temp->ai_addrlen) == -1) {
                close(socket_fd);
                perror("server: bind");
                continue;
            }
        }
        else{
            if (connect(socket_fd, temp->ai_addr, temp->ai_addrlen) == -1) {
                close(socket_fd);
                perror("client: connect");
                continue;
            }
        }
        break;
    }
    if (temp == NULL) {
        fprintf(stderr, "socket creation failed\n");
        return -3;
    }
    freeaddrinfo(servinfo);

    return socket_fd;
}

/*
int auth_course(): Find if the string "code" is the course number in the string "str". e.g.,
                   char *str = "361", char *code = "CS361,..." => auth_course(str, code) = 1
*/
int auth_course(char *str, char *code){
    str += 2;
    return strncmp(str, code, strlen(code)) ? 0 : str[strlen(code)] == ',';
}

/*
void find_course(): Find if the string "code" is the file "fp" of subject "subject". e.g.,
                    char *code = "361", FILE *fp = "cs.txt", char *subject = "CS" => found
                    If found, the string "code" will be the information required by client. e.g.,
                    code <= "The course ... found: CS361,..." if the server received a pure number (e.g., 361)
                    or code <= "The ... of CS361 is ..." if the server received a number with a suffix (e.g., 361C)
*/
void find_course(char *code, FILE *fp, char *subject){
    char str[100], token[2] = ",";
    rewind(fp);
    int course_ctgr = 0;
    char *information_ptr;
    if(code[strlen(code)-1] >= 'A'){
        course_ctgr = code[strlen(code)-1] - 'A' + 1;
        code[strlen(code)-1] = 0;
    }
    printf("The Server%s received a request from the Main Server about %s%s\n", subject, subject, code);
    while(fgets(str, 100, fp) != NULL){
        if(auth_course(str, code)){
            if(course_ctgr == 0){
                strcpy(code, str);
                if(code[strlen(code)-1]!= '\n')strcat(code, "\n");
                information_ptr = code;
                while(*information_ptr != ',')information_ptr++;
                *information_ptr = ':';
                printf("The course information has been found: %s", code);
                return;
            }
            else{
                strcpy(code, "The ");
                strcat(code, course_category_list[course_ctgr-1]);
                strcat(code, " of ");
                strcat(code, strtok(str, token));
                strcat(code, " is ");
                for(int i = 0; i < course_ctgr-1; i++)strtok(NULL, token);
                strcat(code, strtok(NULL, token));
                return;
            }
        }

    }
    strcpy(str, subject);
    strcat(str, code);
    char fail[100] = "Didn't find the course: ";
    strcat(fail, str);
    strcpy(code, fail);
    strcat(code, "\n");   
    printf("%s", code);
}