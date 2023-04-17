#ifndef IPKCP_H
#define IPKCP_H
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
#define SERVER 0
#define CLIENT 1

#define MAXLINE 255
#define MINLENGTH 7

extern char server_address[MAXLINE];
extern char server_port[MAXLINE];
extern bool server_mode;

extern int sockfd;
extern struct sockaddr_in server_addr;

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


void exitError(char* errorMessage);

void handleMode(char* mode);

void parseArguments(int argc, char *argv[]);

int setupSocket();

struct sockaddr_in setupAdress(int sockfd, bool role);

void connectTCP(bool role);


#endif