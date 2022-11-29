#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include "functions.h"

//All send() and recv() functions are modified from Beej's Guide to Network Programming, from server.c & client.c

/*
int auth_true(): to find if the given string is not empty and has only digits.
e.g. auth_true("450") <= 1;
     auth_true("450L") <= 0;
     auth_true("") <= 0;
*/
int auth_true(char *str){
    char *temp = str;
    while(*str >= '0' && *str <= '9')str++;
    return *str == 0 && temp != str;
}

/*
Three kinds of requests in this program:

1. Password Authentication; structure:
'X' + "username" + ',' + "password"; 'X' = 'A' + Attempts Remaining (e.g., 'C' -> 1st attempt, 'A' -> 3rd attempt)

2. Multiple-Course Query; structure:
'X' + "username" + ' ' + "subject indicator" + ' ' + course numbers * "course code without subject";
'X' = 'Q' + course numbers; (e.g., 'S' -> 2 courses, 'T' -> 3 courses)
"subject indicator": e.g., "100" means that 1st course is CS course, while 2nd & 3rd course is EE course.  

3. Single-Course Query with Categories; structure:
'X' + "username" + ' ' + "subject indicator" + ' ' + "course code without subject"
'X' = 'Q' - category index (e.g. Credit -> 0, Days -> 2)
*/
int main(int argc, char* argv[]){
    char PORT[6] = "25";
    char *reason[] = {"Password does not match", "Username Does not exist"};
    struct sockaddr_storage my_addr;
    socklen_t sin_size = 100;
    char username[52], password[52], buf[MAXDATASIZE];
    int sock_fd, numbytes;
    char purpose[] = "D";

    char course_input[61] = {0}, *ptr, ch, course_ctgr[11], course_buf[60], query_ctgr[11];
    int real_num, ctgr_indicator;
    printf("The client is up and running.\n");
    strcat(PORT, L3D);
    while(*purpose < 'E'){
        memset(username, 0, 51);
        memset(password, 0, 51);
        printf("Please enter the username:\t");
        ptr = username;
        while((ch = getchar()) != '\n')if(ptr - username < 50)*ptr++ = ch;//The input maximum is 50 characters
        printf("Please enter the password:\t");
        ptr = password;
        while((ch = getchar()) != '\n')if(ptr - password < 50)*ptr++ = ch;//The input maximum is 50 characters
        
        (*purpose)--;

        //If the length of username/password < 5, consider it wrong without authentication
        if(strlen(username) < 5){
            printf("Authentication failed: %s\nAttempts remaining: %d\n", reason[1], *purpose - 'A');
            if(*purpose == 'A'){
                printf("Authentication Failed for 3 attempts. Client will shut down.\n");
                return 0;
            }
            continue;
        }
        if(strlen(password) < 5){//even if the username does not exist, it will act as a wrong password under this situation
            printf("Authentication failed: %s\nAttempts remaining: %d\n", reason[0], *purpose - 'A');
            if(*purpose == 'A'){
                printf("Authentication Failed for 3 attempts. Client will shut down.\n");
                return 0;
            }
            continue;
        }

        sock_fd = open_socket(PORT, 1, 0, 0);//a TCP socket, as a client
        assert(sock_fd >= 0);

        strcpy(buf, purpose);
        strcat(buf, username);
        strcat(buf, ",");
        strcat(buf, password);
        send(sock_fd, buf, strlen(buf), 0);
        printf("%s sent an authentication request to the main server.\n", username);

        numbytes = recv(sock_fd, buf, MAXDATASIZE-1, 0);
        assert(numbytes != -1);

        buf[numbytes] = 0;
        getsockname(sock_fd, (struct sockaddr *)&my_addr, &sin_size);

        printf("%s received the result of authentication using TCP over port %d. Authentication ", username, ntohs(((struct sockaddr_in *)&my_addr)->sin_port));
        if(*buf != '0'){
            printf("failed: %s\nAttempts remaining: %d\n", reason[*buf - '1'], *purpose - 'A');
            //e.g., if result in serverC is '2', the output will be reason[1];
            if(*purpose == 'A'){
                printf("Authentication Failed for 3 attempts. Client will shut down.\n");
                close(sock_fd);
                return 0;
            }
        }
        else{
            printf("is successful.\n");
            *purpose = 'Q';
        }
        buf[0] = 0;
        close(sock_fd);
    }


    while(1){
        memset(course_ctgr, 0, 11);
        memset(course_buf, 0, 60);
        memset(course_input, 0, 60);
        real_num = 0;
        ptr = course_input;
        printf("Please enter the course code to query: ");
        while((ch = getchar()) != '\n')if(ptr - course_input < 60)*ptr++ = ch;
        
        char *real, token[2] = " ";
        real = strtok(course_input, token);
        while(real != NULL){
            ptr = real + 2;
            if(!strncmp(real, "CS", 2))course_ctgr[real_num] = '1';
            if(!strncmp(real, "EE", 2))course_ctgr[real_num] = '0';
            if(course_ctgr[real_num]){
                if(auth_true(ptr)){
                    strcat(course_buf, " ");
                    strcat(course_buf, ptr);
                    real_num++;
                }
                else course_ctgr[real_num] = 0;
            }
            real = strtok(NULL, token);
        }
        
        ctgr_indicator = 0;
        if(real_num == 0){
            printf("Course Input Error. Please Retry.\n\n-----Start a new request-----\n");
            continue;
        }
        else if(real_num == 1){
            memset(query_ctgr, 0, 11);
            ptr = query_ctgr;
            printf("Please enter the category (Credit / Professor / Days / CourseName): ");
            while((ch = getchar()) != '\n')if(ptr - query_ctgr < 10)*ptr++ = ch;
            for(int i = 0; i < course_category_size; i++)if(strcmp(query_ctgr, course_category_list[i]) == 0){
                ctgr_indicator = i+1;
                break;
            }
            if(ctgr_indicator == 0)printf("Category Input Error. All categories will be displayed.\n");//The segment will be in structure 2 instead of 3.
        }
        *purpose = 'Q' + real_num - ctgr_indicator;
        sock_fd = open_socket(PORT, 1, 0, 0);
        assert(sock_fd >= 0);
        getsockname(sock_fd, (struct sockaddr *)&my_addr, &sin_size);

        strcpy(buf, purpose);
        strcat(buf, username);
        strcat(buf, " ");
        strcat(buf, course_ctgr);
        strcat(buf, course_buf);
        if(send(sock_fd, buf, strlen(buf), 0) == -1)perror("send");
        if(real_num > 1)printf("%s sent a request with multiple CourseCode to the main server.\n", username);
        else printf("%s sent a request to the main server.\n", username);
        buf[0] = 0;
        numbytes = recv(sock_fd, buf, MAXDATASIZE-1, 0);
        assert(numbytes != -1);
        printf("%s received the response from the Main server using TCP over port %d.\n", username, ntohs(((struct sockaddr_in *)&my_addr)->sin_port));
        if(ctgr_indicator == 0)printf("CourseCode: Credits, Professor, Days, Course Name\n");
        printf("%s\n\n-----Start a new request-----\n", buf);
        memset(buf, 0, MAXDATASIZE);
        close(sock_fd);
    }

    return 0;
    
}