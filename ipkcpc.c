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
bool server_mode;

typedef struct{
    uint8_t opcode;
    uint8_t payloadLength;
    char payload[100];
} message_t;

typedef struct{
    uint8_t opcode;
    u_int8_t status;
    u_int8_t payloadLength;
    char payload[100];
} response_t;

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
            strcpy(server_address, argv[i + 1]);
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
    fprintf(stderr, "Server hostname: %s, port: %s, mode: %d\n", server_address, server_port, server_mode);
}

int setupSocket()
{
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
    return sockfd;
}

struct sockaddr_in setupAdress()
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0)
    {
        exitError("Invalid address\n");
    }
    fprintf(stderr, "Adress set\n\n");
    return server_addr;
}

void createMessage(message_t *message)
{
    fprintf(stderr, "Enter message: ");
    fgets(message->payload, 100, stdin);
    fprintf(stderr, " %s", message->payload);
    message->opcode = '\x00';
    message->payloadLength = strlen(message->payload) - 1;
    //debug
    if (message->payload[0] == 'x')
    {
        fprintf(stderr, "\n");
        exit(0);
    }
}

void sendMessage(int sockfd, struct sockaddr_in server_addr, message_t message)
{
    if (sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        exitError("Error sending message\n");
    }
    fprintf(stderr, "Socket sent\n");
}

void receiveMessage(int sockfd, response_t response)
{
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr); 

    if (recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr*)&from_addr, &from_len) < 0)
    {
        exitError("Error receiving message\n");
    }
    else
    {
        printf("Received message: %hhu %hhu %hhu\n", response.opcode, response.status, response.payloadLength);
        response.payload[response.payloadLength] = '\0';
        printf("payload: %s\n\n", response.payload);
    }
}

int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    int sockfd = setupSocket();
    struct sockaddr_in server_addr = setupAdress();
    message_t message;
    response_t response;

    while(true)
    {
        createMessage(&message);
        
        sendMessage(sockfd, server_addr, message);
        
        receiveMessage(sockfd, response);
    }
    return 0;
}