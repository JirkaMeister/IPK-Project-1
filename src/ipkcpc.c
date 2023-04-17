/*
IPK Projekt 1 
Soubor: ipkcpc.c
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


void formatTcpMessage(char *messageTcp)
{
    for (int i = 0; i < strlen(messageTcp); i++)
    {
        if (messageTcp[i] == '\n')
        {
            messageTcp[i + 1] = '\0';
            return;
        }
    }
}

void createMessage(message_t *messageUDP, char *messageTCP)
{
    if (server_mode == UDP)
    {
        fgets(messageUDP->payload, MAXLINE, stdin);
        messageUDP->opcode = '\x00';
        messageUDP->payloadLength = strlen(messageUDP->payload);
    }
    else if (server_mode == TCP)
    {
        fgets(messageTCP, MAXLINE, stdin);
        for (int i = 0; i < strlen(messageTCP); i++)
        {
            if (messageTCP[i] == '\r')
            {
                messageTCP[i] = '\n';
                messageTCP[i + 1] = '\0';
            }
        }
    }
    
}

void sendMessage(message_t message, char *messageTcp)
{
    if (server_mode == UDP)
    {
        if (sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
            exitError("Error sending message\n");
        }
    }
    else if (server_mode == TCP)
    {
        if (send(sockfd, messageTcp, strlen(messageTcp), 0) < 0)
        {
            exitError("Error sending message\n");
        }
    }
}

void receiveMessage()
{
    if (server_mode == UDP)
    {
        response_t response;
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        if (recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr*)&from_addr, &from_len) < 0)
        {
            exitError("Error receiving message\n");
        }
        else
        {
            response.payload[response.payloadLength] = '\0';
            if (response.status == 0)
            {
                printf("OK:%s\n", response.payload);
            }
            else
            {
                printf("ERR:%s\n", response.payload);
            }
        }
    }
    else if (server_mode == TCP)
    {
        char responseTcp[MAXLINE];
        if (recv(sockfd, responseTcp, strlen(responseTcp), 0) < 0)
        {
            exitError("Error receiving message\n");
        }
        else
        {
            formatTcpMessage(responseTcp);
            printf("%s", responseTcp);
        }

        if (strcmp(responseTcp, "RESULT") == 0)
        {
            
            if (recv(sockfd, responseTcp, strlen(responseTcp), 0) < 0)
            {
                exitError("Error receiving message\n");
            }
            else
            {
                formatTcpMessage(responseTcp);
                printf("%s", responseTcp);
            }
        }
        else if (strcmp(responseTcp, "BYE\n") == 0)
        {
            close(sockfd);
            exit(0);
        }
    }
    
}

void closeConnection()
{
    message_t message;
    sendMessage(message, "BYE\n");
    receiveMessage(sockfd);
    close(sockfd);
}

void handle_sigint()
{
    if (server_mode == TCP)
    {
        closeConnection(CLIENT);
    }
}

int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    signal(SIGINT, handle_sigint);

    sockfd = setupSocket();
    server_addr = setupAdress(sockfd, CLIENT);
    connectTCP(CLIENT);

    message_t messageUDP;
    char messageTCP[MAXLINE];
    char firstChar = getchar();

    while(firstChar != EOF)
    {
        ungetc(firstChar, stdin);
        createMessage(&messageUDP, messageTCP);
        sendMessage(messageUDP, messageTCP);
        receiveMessage();
        firstChar = getchar();
    }
    if (server_mode == TCP)
    {
        closeConnection();
    }
    return 0;
}