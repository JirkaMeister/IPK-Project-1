#ifndef IPKCP_H
#define IPKCP_H
#include <unistd.h>

#define UDP 0
#define TCP 1

#define MAXLINE 255


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

#endif