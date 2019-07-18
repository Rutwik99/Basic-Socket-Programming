#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#define PORT 8080
#define MAX_SIZE 10240

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread,input_len;
    struct sockaddr_in serv_addr;
    char buffer[10240] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    // send(sock , hello , strlen(hello) , 0 );  // send the message.
    // printf("I just sent a request to the server .\n");

    printf("***************\n CLIENT START  \n***************\n");
    // valread = read( sock , buffer, 10240);  // receive message back from server, into the buffer
    // printf("Server :%s\n",buffer );
    printf("Please enter your commands\n");

    char *args[100];
    char input[100];
    char input1[100];
    int com_type;
    char *command;
    char file_name[100];
    int no_cmds = 0;
    int cnt1 = 1;
    int divisor;

    while (1)
    {
        strcpy(input, "");
        strcpy(input1, "");
        strcpy(buffer, "");
        no_cmds = 0;
        cnt1 = 1;
        printf("\n>>> ");
        fgets(input1, 100, stdin);
        strcpy(input, input1);
        args[0] = strtok(input1, " \n");
        no_cmds++;
        while((args[cnt1] = strtok(NULL, " \n")) != NULL)
        {
            cnt1++;
            no_cmds++;
        }
    
        args[cnt1] = NULL;
        if(strcmp(args[0], "listall") == 0)
        {
            if(no_cmds > 1)
            {
                printf("There are no arguments for 'listall'\n");
                exit(1);
            }
            com_type = 1;
        }

        // SEND COMMAND
        if(strcmp(args[0], "send") == 0)
        {
            if(no_cmds > 2)
            {
                printf("There is one argument for 'send'\n");
                exit(1);
            }
            if(no_cmds == 1)
            {
                printf("Please enter a filename\n");
                exit(1);
            }
            strcpy(file_name, args[1]);
            com_type = 0;
        }

        // QUIT COMMAND
        if(strcmp(args[0], "quit") == 0)
        {
            break;
        }

        send(sock , input , strlen(input) , 0 );  // send the message.

        valread = read( sock , buffer, 10240);  // receive message back from server, into the buffer
        
        // LISTALL COMMAND
        if(com_type == 1)
        {
            printf("%s", buffer);
        }

        // SEND COMMAND
        if(com_type == 0)
        {
            int fp1;
            if(strcmp(buffer, "read_error") == 0)
            {
                printf("No File Found in Server\n");
                exit(1);
            }
            else 
            {
                fp1 = open(file_name, O_CREAT | O_WRONLY, 0777);
                if(fp1 < 0)
                {
                    perror("Error Writing to File");
                }

                write(fp1, buffer, strlen(buffer));
                close(fp1);
            }
        }
    }
     return 0;
}