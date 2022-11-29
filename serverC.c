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

/*
int up_cmp(): find if two strings are the same except the last character.
              The return value is:
                0 if the two strings are the same except the last character of str is 13('\n')
                1 if the two strings are different, and they differ after a same ',' (which segregates username and password)
                2 if the two strings are different, and they differ before a same ','
*/
int up_cmp(char *user, char *str){
    int i = 2;
    while(*user++ == *str++){
        if(*user == ',' && *str == ',')i = 1;//when(*user == ',' && *str == ','), it means the usernames are the same
        if(*user == 0 || *str == 13)break;
    }
    
    return (*user || *str != 13) ? i : i-1;
}

/*
int auth(): with a set of usernames and passwords in a file "fp",
            find if the given username and password is in this file.
            The return value is:
                0 if the username is in the file with the corresponding password
                1 if the username is in the file but the password is wrong
                2 if the username is not in the file
*/
int auth(char *user, FILE *fp){
    rewind(fp);
    char str[101];
    while(fgets(str, 100, fp) != NULL)switch (up_cmp(user, str)){
        case 0: return 0;
        case 1: return 1;
        default: break;
    }
    return 2;
}

int main(int argc, char* argv[]){
    int self_fd, main_fd;
    char ser_c[6] = "21", m_udp[6] = "24", res[2] = "0";
    int numbytes;
    FILE *fp = fopen("cred.txt", "r");
    assert(fp != NULL);
    char buf[MAXDATASIZE], user[102];

    self_fd = open_socket(strcat(ser_c, L3D), 0, 1, 1);//a UDP socket, as the server
    assert(self_fd >= 0);
    printf("The ServerC is up and running using UDP on port %s.\n", ser_c);

    main_fd = open_socket(strcat(m_udp, L3D), 0, 0, 0);//a UDP socket, as a client
    while(1){
        res[0] = '0';
        numbytes = recv(self_fd, buf, MAXDATASIZE-1 , 0);
        assert(numbytes != -1);
        printf("The ServerC received an authentication request from the Main Server.\n");
        buf[numbytes] = 0;
        
        strcpy(user, buf);
        res[0] += auth(user, fp);
        //res[0] will be the character form of the result of auth(); i.e. 0 -> '0', 1 -> '1', 2 -> '2'
        
        if(send(main_fd, res, 1, 0) == -1)perror("send");
        printf("The ServerC finished sending the response to the Main Server.\n");
        buf[0] = 0;
    }
    return 0;
}