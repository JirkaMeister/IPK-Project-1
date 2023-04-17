#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

int setupSocket()
{
    int family = AF_INET;
    int type;
    int protocol = 0;
    if (server_mode == UDP)
    {
        type = SOCK_DGRAM;
    }
    else if (server_mode == TCP)
    {
        type = SOCK_STREAM;
    }
    int sockfd = socket(family, type, protocol);
    if (sockfd < 0)
    {
        exitError("Error creating socket\n");
    }
    return sockfd;
}

struct sockaddr_in setupAdress(int sockfd, bool role)
{
    struct hostent *server = gethostbyname(server_address);
    if (server == NULL)
    {
        exitError("Error resolving hostname\n");
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));

    if (role == CLIENT)
    {
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0)
        {
            exitError("Invalid address\n");
        }
    }
    else if (role == SERVER)
    {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    
        if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            exitError("Error binding socket\n");
        }
    }

    return server_addr;
}

void connectTCP(bool role)
{
    if (server_mode == TCP && role == CLIENT)
    {
        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
            exitError("Error connecting to server\n");
        }
    }
    else if (server_mode == TCP && role == SERVER)
    {
        int maxConnections = 3;
        if (listen(sockfd, maxConnections) < 0)
        {
            exitError("Error listening on socket\n");
        }
    }
}

