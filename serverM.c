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

#define BACKLOG 10

//All send() and recv() functions are modified from Beej's Guide to Network Programming, from server.c & client.c

//void sigchld_handler(): This function is copied from Beej's Guide to Network Programming, from server.c
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

/*
int can_encrypt(): Find out the category of goven character. e.g.,
                   can_encrypt('5') <= 1;
                   can_encrypt('A') <= 2;
                   can_encrypt('a') <= 3;
                   can_encrypt('@') <= 0;
*/
int can_encrypt(char ch){
    if(ch >= 48 && ch <= 57)return 1;
    if(ch >= 65 && ch <= 90)return 2;
    if(ch >= 97 && ch <= 122)return 3;
    return 0;
}

void encrypt(char *p){//encrypt the username and password
    do switch (can_encrypt(*p)){
            case 1: *p = (*p-'0'+4)%10 + '0';break;
            case 2: *p = (*p-'A'+4)%26 + 'A';break;
            case 3: *p = (*p-'a'+4)%26 + 'a';break;
            default: break;
        }    
    while(*p++);
}

int main(int argc, char* argv[]){
    int C_fd, CS_fd, EE_fd, udp_fd, tcp_fd, client_fd, status;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int numbytes;
    char 
        ser_c[6] = "21",
        ser_cs[6] = "22",
        ser_ee[6] = "23",
        m_udp[6] = "24",
        m_tcp[6] = "25";
    char buf[MAXDATASIZE], account[102] = {0};
    char *sp;
    int is_CS;
    
    char *subject[] = {"EE", "CS"}, *usrp;
    
    udp_fd = open_socket(strcat(m_udp, L3D), 0, 1, 1);//a UDP socket, as the server
    assert(udp_fd >= 0);
    
    tcp_fd = open_socket(strcat(m_tcp, L3D), 1, 1, 1);//a TCP socket, as the server
    assert(tcp_fd >= 0);

    C_fd = open_socket(strcat(ser_c, L3D), 0, 0, 0);//a UDP socket, as a client
    assert(C_fd >= 0);

    CS_fd = open_socket(strcat(ser_cs, L3D), 0, 0, 0);//a UDP socket, as a client
    assert(CS_fd >= 0);

    EE_fd = open_socket(strcat(ser_ee, L3D), 0, 0, 0);//a UDP socket, as a client
    assert(EE_fd >= 0);

//The following codes are directly copied from Beej's Guide to Network Programming, from server.c
    if (listen(tcp_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("The main server is up and running.\n");

    while(1) { 
        sin_size = sizeof their_addr;
        client_fd = accept(tcp_fd, (struct sockaddr *)&their_addr, &sin_size);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        

        if (!fork()) { 
            close(tcp_fd);
//The copied codes end here
            numbytes = recv(client_fd, buf, MAXDATASIZE-1, 0);
            assert(numbytes != -1);
            buf[numbytes] = 0;
            sp = buf;
            
            if(*sp++ < 'D'){
                int i = 0;
                memset(account, 0, 102);
                while(*sp != ',')account[i++] = *sp++;//now "account" is the username (before ',')
                printf("The main server received the authentication for %s using TCP over port %s.\n", account, m_tcp);
                while(*sp)account[i++] = *sp++;//now "account" is the username+password (segregated by ',')
                account[i] = 0;
                buf[0] = 0;

                encrypt(account);

                if(send(C_fd, account, strlen(account), 0) == -1)perror("send");
                printf("The main server sent an authentication request to serverC.\n");

                numbytes = recv(udp_fd, buf, MAXDATASIZE-1, 0);
                assert(numbytes != -1);
                printf("The main server received the result of the authentication request from ServerC using UDP over port %s.\n", m_udp);

                buf[numbytes] = 0;
                if(send(client_fd, buf, strlen(buf), 0) == -1)perror("send");
                printf("The main server sent the authentication result to the client.\n");
            }
            else{
                char ctgr[11], *code, output[100], token[2] = " ", buf_temp[MAXDATASIZE] = {0};
                int course_num = *buf-'Q';
                
                usrp = sp;
                sp = strchr(sp, ' ');
                *sp++ = 0;
                //separate the username and other parts of message
                //e.g. Tjames 100 356 450 658 becomes
                //     |usrp |sp
                //Tjames(\0)100 356 450 658
                //|usrp     |sp
                
                if(course_num > 0){
                    printf("The main server received the authentication for %s to query the following courses using TCP over port %s.\n", usrp, m_tcp);
                    code = strtok(sp, token);
                    for(int i = 0; code != NULL; i++){
                        memset(output, 0, 100);
                        if(i == 0){
                            strcpy(ctgr, code);
                            code = strtok(NULL, token);
                            continue;
                        }
                        if(ctgr[i-1] == '1'){
                            printf("CS%s: ", code);
                            if(send(CS_fd, code, strlen(code), 0) == -1)perror("send");
                            printf("The main server sent a request to serverCS, ");
                            numbytes = recv(udp_fd, output, MAXDATASIZE-1, 0);
                            assert(numbytes != -1);
                            strcat(buf_temp, output);
                            printf("received the response from serverCS using UDP over port %s.\n", m_udp);
                        }
                        else{
                            printf("EE%s: ", code);
                            if(send(EE_fd, code, strlen(code), 0) == -1)perror("send");
                            printf("The main server sent a request to serverEE, ");
                            numbytes = recv(udp_fd, output, MAXDATASIZE-1, 0);
                            assert(numbytes != -1);
                            strcat(buf_temp, output);
                            printf("received the response from serverEE using UDP over port %s.\n", m_udp);
                        }
                        code = strtok(NULL, token);
                    }
                }
                else{
                    memset(output, 0, 100);
                    course_num = -course_num;//course_num = *buf-'Q', now it is the opposite number of course category index.
                    is_CS = (*sp == '1');
                    sp += 2;
                    printf("The main server received from %s to query course %s%s about %s using TCP over port %s.\n", usrp, subject[is_CS], sp, course_category_list[course_num], m_tcp);
                    *token = 'A' + course_num;
                    strcat(sp, token);
                    if(is_CS){
                        if(send(CS_fd, sp, strlen(sp), 0) == -1)perror("send");
                        printf("The main server sent a request to serverCS.\n");
                    }
                    else{
                        if(send(EE_fd, sp, strlen(sp), 0) == -1)perror("send");
                        printf("The main server sent a request to serverEE.\n");
                    }
                    numbytes = recv(udp_fd, output, MAXDATASIZE-1, 0);
                    assert(numbytes != -1);
                    strcat(buf_temp, output);
                }
                if(send(client_fd, buf_temp, strlen(buf_temp), 0) == -1)perror("send");
                printf("The main server sent the query information to the client.\n");
            }
            close(client_fd);
            exit(0);
        }
        close(client_fd); 
    }

    return 0;
}
