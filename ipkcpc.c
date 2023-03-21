#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define TCP 0
#define UDP 1

char server_address[100];
char server_port[100];
char message[100];
bool server_mode;

void exitError(char* errorMessage)
{
    fprintf(stderr, "%s", errorMessage);
    exit(1);
}

void handleIP(char* ip)
{
    /* int i = 0;
    int count = 0;
    for (i = 0; i < strlen(ip); i++)
    {
        if (ip[i] == '.')
        {
            count++;
        }
        else if (ip[i] < '0' || ip[i] > '9')
        {
            exitError("Invalid IP address\n");
        }
    }
    if (count != 3)
    {
        exitError("Invalid IP address\n");
    } */
    strcpy(server_address, ip);
}

void handlePort(char* port)
{
    int i = 0;
    for (i = 0; i < strlen(port); i++)
    {
        if (port[i] < '0' || port[i] > '9')
        {
            exitError("Invalid port\n");
        }
    }
    strcpy(server_port, port);
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
    if (argc != 7)
    {
        exitError("Invalid number of arguments\n");
    }
    handleIP(argv[2]);
    handlePort(argv[4]);
    handleMode(argv[6]);
}
int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    printf("Server hostname: %s, port: %s, mode: %d\n", server_address, server_port, server_mode);

    printf("Enter message: ");
    fgets(message, 100, stdin);
    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;
    if (server_mode == UDP)
    {
        type = SOCK_DGRAM;
    }
    int sockfd = socket(family, type, protocol);
    if (sockfd < 0)
    {
        exitError("Error creating socket\n");
    }
    fprintf(stderr, "Socket created\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0)
    {
        exitError("Invalid address\n");
    }
    fprintf(stderr, "Adress set\n");
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        exitError("Error sending message\n");
    }
    fprintf(stderr, "Socket sent\n");

    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    char buffer[100];
    if (recvfrom(sockfd, buffer, 100, 0, (struct sockaddr*)&from_addr, &from_len) < 0)
    {
        exitError("Error receiving message\n");
    }
    else
    {
        printf("Received message: %s\n", buffer);
    }

    return 0;
}