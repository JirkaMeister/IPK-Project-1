/*
IPK Projekt 2
Soubor: ipkcpd.c
Autor: Jiri Charamza
Login: xchara04
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#include "ipkcpUtils.h"


char server_address[MAXLINE];
char server_port[MAXLINE];
bool server_mode;

int sockfd;
struct sockaddr_in server_addr;


void exitError(char* errorMessage)
{
    fprintf(stderr, "%s", errorMessage);
    exit(1);
}

void handleMode(char* mode)
{
    if (strcmp(mode, "tcp") == 0)
    {
        server_mode = TCP;
    }
    else if (strcmp(mode, "udp") == 0)
    {
        server_mode = UDP;
    }
    else
    {
        exitError("Invalid server mode\n");
    }
}

void parseArguments(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        printf("USAGE:\n");
        printf("\t./ipkcpc [option] [argument]\n\n");
        printf("OPTIONS:\n");
        printf("\t--help\t\tDisplay this help menu\n");
        printf("\t-h\t\tServer IPv4 adress (default: 0.0.0.0)\n");
        printf("\t-p\t\tServer port (default: 2023)\n");
        printf("\t-m\t\tServer mode - TCP or UDP\n\n");
        exit(0);
    }
    else if (argc != 7)
    {
        exitError("Invalid number of arguments\n");
    }
    for(int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            if (strcmp(argv[i + 1], "localhost") == 0)
            {
                strcpy(server_address, "0.0.0.0");
            }
            else
            {
                strcpy(server_address, argv[i + 1]);
            }
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            strcpy(server_port, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            handleMode(argv[i + 1]);
        }
        else
        {
            exitError("Invalid argument\n");
        }
    }
}


int main(int argc, char *argv[])
{
    parseArguments(argc, argv);

    if (server_mode == TCP)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    if (sockfd < 0)
    {
        exitError("Error creating socket\n");
    }

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));
    server_addr.sin_addr.s_addr = inet_addr(server_address);

    struct sockaddr *address = (struct sockaddr *) &server_addr;
    int address_size = sizeof(server_addr);

     if (bind(sockfd, address, address_size) < 0)
    {
        exitError("Error binding socket\n");
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    struct sockaddr *client_address = (struct sockaddr *) &client_addr;

    while(true)
    {
        message_t message;
        response_t response;
        fprintf(stderr, "Waiting for message");
        int bytes_received = recvfrom(sockfd, &message, sizeof(message), 0, client_address, &client_addr_size);
        if (bytes_received < 0)
        {
            exitError("Error receiving message\n");
        }
        else
        {
            fprintf(stderr, "Received message");
        }
        int bytes_sent = sendto(sockfd, &response, sizeof(response), 0, client_address, client_addr_size);
        if (bytes_sent < 0)
        {
            exitError("Error sending response\n");
        }

    }
}