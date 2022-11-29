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
#include <assert.h>
#include "functions.h"

//All send() and recv() functions are modified from Beej's Guide to Network Programming, from server.c & client.c

int main(int argc, char* argv[]){
    int self_fd, main_fd;
    char ser_ee[6] = "23", m_udp[6] = "24";
    int numbytes;
    FILE *fp = fopen("ee.txt", "r");
    assert(fp != NULL);
    char buf[MAXDATASIZE];

    self_fd = open_socket(strcat(ser_ee, L3D), 0, 1, 1);//a UDP socket, as the server
    assert(self_fd >= 0);
    printf("The ServerEE is up and running using UDP on port %s.\n", ser_ee);

    main_fd = open_socket(strcat(m_udp, L3D), 0, 0, 0);//a UDP socket, as a client
    assert(main_fd >= 0);
    while(1){
        numbytes = recv(self_fd, buf, MAXDATASIZE-1 , 0);
        assert(numbytes != -1);
        buf[numbytes] = 0;       
        find_course(buf, fp, "EE");//find if this course is a EE course
        if(send(main_fd, buf, strlen(buf), 0) == -1)perror("send");
        printf("The ServerEE finished sending the response to the Main Server.\n");
        buf[0] = 0;
    }
    return 0;
}