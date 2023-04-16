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

#define UDP 0
#define TCP 1

#define MAXLINE 255

char server_address[MAXLINE];
char server_port[MAXLINE];
bool server_mode;

int sockfd;
struct sockaddr_in server_addr;

typedef struct{
    uint8_t opcode;
    uint8_t payloadLength;
    char payload[MAXLINE];
} message_t;

typedef struct{
    uint8_t opcode;
    u_int8_t status;
    u_int8_t payloadLength;
    char payload[MAXLINE];
} response_t;

void exitError(char* errorMessage)
{
    fprintf(stderr, "%s", errorMessage);
    exit(1);
}


int main(int argc, char *argv[])
{
    
}