#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include<dirent.h>
#define PORT 8080
#define BUFFERSIZE 10240
char delimiter[]="  \t\r\n\v\f";
DIR *d;
struct dirent *dir;


int main(int argc, char const *argv[])
{
    int main_socket, new_socket, valread,client_socket[10], max_clients=10,activity,max_sd,sd;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[10240] = {0};
    fd_set readfds;
    // char *welcome_message = "Hello from server .Please enter the command you want";

    for(int m=0;m<max_clients;m++){
        client_socket[m]=0;
    }

    // Creating socket file descriptor
    if ((main_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(main_socket, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(main_socket, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
        //add main socket to set
        FD_SET(main_socket, &readfds);
        max_sd = main_socket;
        //add child sockets to set
        for(int m = 0;m < max_clients; m++)
        {
            //socket descriptor
            sd = client_socket[m];
            //if it's a valid socket descriptor then we add it to read list
            if(sd > 0)
            {
                FD_SET(sd,&readfds);
            } 
            //highest file descriptor need it for select function .
            if(sd > max_sd)
            {
                max_sd = sd;
            }
        }
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select(max_sd + 1 , &readfds, NULL, NULL, NULL);   
       
        if((activity < 0) && (errno!=EINTR))
        {   
            printf("select error");   
        }   
             
        //If something happened on the main socket ,  
        //then its an incoming connection  
        if (FD_ISSET(main_socket, &readfds))   
        {   
            // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
            if ((new_socket = accept(main_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
           
            //send new connection greeting message  
            // if( send(new_socket , welcome_message , strlen(welcome_message) , 0 ) != strlen(welcome_message) )   
            // {   
            //     perror("send");   
            // }   
            for(int l = 0;l < max_clients; l++)
            {
                if(client_socket[l] == 0){
                    client_socket[l] = new_socket;
                    printf("Adding to list of sockets as %d\n", l);
                    break;
                }
            }     
        }

        char total_filelist[1000];
        char buffer1[10240];
        char *command;
        char *filename;
        char file_data[10000];
        int data_length;
        int divisor;
        int remainder;
        int character;
		
        for(int k = 0; k < max_clients; k++)
        {
            sd = client_socket[k];
            char buffer[10240] = {0};
            if(FD_ISSET(sd,&readfds))
            {
                valread = read(sd, buffer, 10240);
                if(valread == 0)
                {
                     //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port)); 
                    //Close the socket and mark as 0 in list for reuse  
                    close(sd);
                    client_socket[k] = 0;
                }
                else
                {
                    strcpy(buffer1, buffer);
                    char *token = strtok(buffer, "\n");
                    token = strtok(buffer, " ");
                    command = strdup(token);
                    token = strtok(NULL," ");


                    // LISTALL COMMAND
                    if(strcmp(command, "listall") == 0)
                    {
                        strcpy(total_filelist, "");
                        struct dirent **filelist;
                        int n;
                        int i;

                        n = scandir(".", &filelist, NULL, alphasort);
                        for(i = 0; i < n; i++)
                        {
                            if((filelist[i]->d_name)[0] != '.')
                            {
                                strcat(filelist[i]->d_name, "\n");
                                strcat(total_filelist, filelist[i]->d_name);
                            }
                            free(filelist[i]);
                        }
                        free(filelist);
                        send(new_socket , total_filelist, strlen(total_filelist), 0);
                    }
                    
                    // SEND COMMAND
                    if(strcmp(command, "send") == 0)
                    {
                        strcpy(file_data, "");
                        int fp1, er;
                        fp1 = open(token, O_RDONLY);
                        if(fp1 < 0)
                        {
                            perror("Error Opening File");
                            strcpy(file_data, "read_error");
                            send(new_socket , file_data, strlen(file_data), 0);
                            exit(1);
                        }
                        data_length = lseek(fp1, 0, SEEK_END);
                        lseek(fp1, 0, SEEK_SET);
                        
                        read(fp1, file_data, data_length);

                        close(fp1);
                        send(new_socket , file_data, strlen(file_data), 0);
				    } 
                }
            }
            
        }
    }
    return 0;
}