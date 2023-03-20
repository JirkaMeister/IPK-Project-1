#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>

#define TCP 0
#define UDP 1

char server_hostname[100];
char server_port[100];
//char message[100];
bool server_mode;

void exitError(char* errorMessage)
{
    fprintf(stderr, "%s", errorMessage);
    exit(1);
}

void handleIP(char* ip)
{
    int i = 0;
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
    }
    strcpy(server_hostname, ip);
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
    printf("Server hostname: %s, port: %s, mode: %d\n", server_hostname, server_port, server_mode);




    return 0;
}