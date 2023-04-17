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
#include <regex.h>
#include <math.h>

#include "ipkcpUtils.h"


char server_address[MAXLINE];
char server_port[MAXLINE];
bool server_mode;

bool helloMessage = true;
int sockfd;
int comm_sockfd;
struct sockaddr_in server_addr;

bool useDouble = false;



int countDecimalPlaces(double number)
{
    int decimalPlaces = 0;
    while (number != (int)number && decimalPlaces < 10)
    {
        number *= 10;
        decimalPlaces++;
    }
    if (decimalPlaces < 1)
    {
        decimalPlaces = 1;
    }
    return decimalPlaces;
}


double solve(char *str, bool *success, bool firstCall)
{
    if (firstCall)
    {
        *success = true;
        useDouble = false;
    }

    regex_t regex;
    int regexMatch;
    char msgbuf[100];
    double result;

    char* pattern = "^\\s*\\(\\s*[+*/-](\\s*[+-]?[0-9]+\\.?[0-9]*([eE][+-]?[0-9]+)?)+\\s*\\)\\s*$";

    regexMatch = regcomp(&regex, pattern, REG_EXTENDED);
    if (regexMatch)
    {
        fprintf(stderr, "Chyba při překladu regulárního výrazu\n");
        exit(1);
    }
    char *EOL = strchr(str, '\n');

    if (EOL != NULL)
    {
        *EOL = '\0';
    }

    char *EOLwin = strchr(str, '\r');

    if (EOLwin != NULL)
    {
        *EOLwin = '\0';
    }

    char cleanString[256] = "\0";
    char nonSpaceSequence[100] = "\0";
    int offset = 0;
    while (sscanf(str + offset, "%s", nonSpaceSequence) == 1)
    {
        sprintf(cleanString + strlen(cleanString), "%s ", nonSpaceSequence);
        offset += strlen(nonSpaceSequence);
        while (str[offset] == ' ')
        {
            offset++;
        }
    }
    strcpy(str, cleanString);

    if (str[0] == ' ')
    {
        strcpy(str, str + 1);
    }
    if (str[1] != '+' && str[1] != '-' && str[1] != '*' && str[1] != '/')
    {
        strcpy(str + 1, str + 2);
    }

    if (strchr(str, 'e') != NULL || strchr(str, 'E') != NULL || strchr(str, '.') != NULL)
    {
        useDouble = true;
    }

    double operand[256];
    int operandCount = 0;
    char initializedOperands[256];

    solveRegex:
    regexMatch = regexec(&regex, str, 0, NULL, 0);


    if (!regexMatch)
    {
        char operator = str[1];
        double operandUnused;
        operandCount = 0;
        str = strchr(str, ' ');
        str++;

        while(true)
        {
            if (sscanf(str, "%lf", &operand[operandCount]) != 1)
            {
                break;
            }
            operandCount++;
            str = strchr(str, ' ');
            if (str == NULL)
            {
                break;
            }
            str++;
        }

        result = operand[0];

        for (int i = 1; i < operandCount; i++)
        {
            switch(operator)
            {
                case '+':
                    result += operand[i];
                    break;
                case '-':
                    result -= operand[i];
                    break;
                case '*':
                    result *= operand[i];
                    break;
                case '/':
                    result /= operand[i];
                    break;
            }
        }
    }
    else if (regexMatch == REG_NOMATCH)
    {
        char leftBracketIndex = -1;
        char rightBracketIndex = -1;
        
        bool inOperand = false;
        bool inBracket = false;
        for (int i = 1; i < strlen(str) - 1; i++)
        {
            if (str[i] >= '0' && str[i] <= '9' && !inOperand && !inBracket)
            {
                operandCount++;
                inOperand = true;
            }
            if (inOperand)
            {
                if (str[i] == ' ')
                {
                    inOperand = false;
                }
            }

            if (str[i] == '(')
            {
                leftBracketIndex = i;
            }
            if (str[i] == ')')
            {
                rightBracketIndex = i;
                break;
            }
        }
        if (leftBracketIndex == -1 || rightBracketIndex == -1)
        {
            *success = false;
            regfree(&regex);
            return 0;
        }
        else
        {
            char tmpString[256];
            strcpy(tmpString, str + leftBracketIndex);
            tmpString[rightBracketIndex - leftBracketIndex + 1] = '\0';
            char rightSideTmp[256];
            strcpy(rightSideTmp, str + rightBracketIndex + 1);

            double result = solve(tmpString, success, false);
            char resultString[64];
            sprintf(resultString, "%.*lf", countDecimalPlaces(result), result);

            int resultStringLength = strlen(resultString);
            strcpy(str + leftBracketIndex, resultString);

            strcpy(str + leftBracketIndex + resultStringLength, rightSideTmp);

            goto solveRegex;
        }
        
    }
    else
    {
        *success = false;
        regfree(&regex);
        return 0;
    }


    regfree(&regex);
    return result;  
}


int gcd(int a, int b) {
    if (b == 0) {
        return a;
    } else {
        return gcd(b, a % b);
    }
}

void to_fraction(double x, int *numerator, int *denominator) {
    int sign = 1;
    if (x < 0) {
        sign = -1;
        x = -x;
    }
    int int_part = (int) x;
    double frac_part = x - int_part;
    int pow10 = 1;
    while ((int) frac_part != frac_part && pow10 <= 100000000) {
        frac_part *= 10;
        pow10 *= 10;
    }
    *numerator = int_part * pow10 + round(frac_part);
    *denominator = pow10;
    int g = gcd(*numerator, *denominator);
    *numerator /= g;
    *denominator /= g;
    *numerator *= sign;
}


response_t handleRequest(message_t messageUDP, char *messageTCP, bool *success)
{
    response_t response;
    double result;

    if (server_mode == UDP)
    {
        response.opcode = '\x01';
        if (messageUDP.opcode == '\x00')
        {
            response.status = '\x00';
        }
        else
        {
            response.status = '\x01';
        }

        for(int i = 0; i < messageUDP.payloadLength; i++)
        {
            if (messageUDP.payload[i] == '\n' || messageUDP.payload[i] == '\r')
            {
                messageUDP.payload[i] = '\0';
                break;
            }
        }
        response.payloadLength = strlen(messageUDP.payload);
        result = solve(messageUDP.payload, success, true);
    }
    else if (server_mode == TCP)
    {
        char *EOL = strchr(messageTCP, '\n');
        if (EOL != NULL)
        {
            *EOL = '\0';
        }

        EOL = strchr(messageTCP, '\r');
        if (EOL != NULL)
        {
            *EOL = '\0';
        }

        if (helloMessage)
        {
            if (strcmp(messageTCP, "HELLO") == 0)
            {
                strcpy(response.payload, "HELLO\n");
                helloMessage = false;
                return response;
            }
            else
            {
                *success = false;
            }
        }
        else
        {
            if (strlen(messageTCP) > 6 && messageTCP[0] == 'S' && messageTCP[1] == 'O' && messageTCP[2] == 'L' && messageTCP[3] == 'V' && messageTCP[4] == 'E' && messageTCP[5] == ' ')
            {
                result = solve(messageTCP + 6, success, true);
            }
            else
            {
                *success = false;
            }
        }
    }
    
    
    if (!(*success))
    {
        if (server_mode == UDP)
        {
            response.status = '\x01';
            strcpy(response.payload, "Could not parse the message");
        }
        else if (server_mode == TCP)
        {
            strcpy(response.payload, "BYE\n");
        }
    }
    else
    {
        if (useDouble)
        {
            sprintf(response.payload, "%.*lf", countDecimalPlaces(result), result);
        }
        else if ((int)result == result)
        {
            sprintf(response.payload, "%d", (int)result);
        }
        else
        {
            int numerator, denominator;
            to_fraction(result, &numerator, &denominator);
            sprintf(response.payload,"%d/%d", numerator, denominator);
        }
    }
    response.payloadLength = strlen(response.payload);
    return response;
}

void handle_sigint()
{
    if (server_mode == TCP)
    {
        if (sockfd != 0)
        {
            close(sockfd);
        }
    }
}

int main(int argc, char *argv[])
{
    parseArguments(argc, argv);

    // Interrupt signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        exitError("Error with setting signal handler\n");
    }

    sockfd = setupSocket();
    int enable = 1;
    int sockNew = 0;
    if (server_mode == TCP)
    {
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)))
        {
            exitError("Error with setting socket options\n");
        }   
    }

    server_addr = setupAdress(sockfd, SERVER);
    connectTCP(SERVER);

    while(true)
    {
        message_t messageUDP;
        char messageTCP[MAXLINE];
        response_t response;
        bool success = true;

        if (server_mode == UDP)
        {
            struct sockaddr_in client_addr;
            socklen_t client_addr_size = sizeof(client_addr);
            struct sockaddr *client_address = (struct sockaddr *) &client_addr;
            
            response.opcode = '\x01';


            if (recvfrom(sockfd, &messageUDP, sizeof(messageUDP), 0, client_address, &client_addr_size) < 0)
            {
                if (errno != EINTR)
                {
                    exitError("Error receiving message\n");
                }
                close(sockfd);
                exit(0);
            }


            // Handle message
            response = handleRequest(messageUDP, messageTCP, &success);

            if (sendto(sockfd, &response, sizeof(response), 0, client_address, client_addr_size) < 0)
            {
                if (errno != EINTR)
                {
                    exitError("Error sending response\n");
                }
                close(sockfd);
                exit(0);
            }
        }
        else if (server_mode == TCP)
        {
            char messageTCP[MAXLINE];
            char responseTCP[MAXLINE];
            pid_t pid;
            
            if (sockNew == 0)
            {
                int addrlen = sizeof(server_addr);
                sockNew = accept(sockfd, (struct sockaddr *)&server_addr, (socklen_t*)&addrlen);
                if (sockNew < 0)
                {
                    if (errno == EINTR)
                    {
                        close(sockfd);
                        exit(0);
                    }
                    else
                    {
                        exitError("Error accepting connection\n");
                    }
                }

                pid = fork();
            }

            if (pid == -1)
            {
                exitError("Error forking\n");
            }
            else if (pid == 0)
            {
                if (recv(sockNew, &messageTCP, sizeof(messageTCP), 0) < 0)
                {
                    if (errno == EINTR)
                    {
                        close(sockNew);
                        exit(0);
                    }
                    else
                    {
                        exitError("Error receiving message\n");
                    }
                }

                response = handleRequest(messageUDP, messageTCP, &success);
                strcpy(responseTCP, response.payload);

                if (strcmp(responseTCP, "HELLO\n") != 0 && strcmp(responseTCP, "BYE\n") != 0 && success == true)
                {
                    char tmp[MAXLINE];
                    strcpy(tmp, responseTCP);
                    strcpy(responseTCP, "RESULT ");
                    strcat(responseTCP, tmp);
                    strcat(responseTCP, "\n");
                    strcat(responseTCP, "\0");
                }


                if (send(sockNew, &responseTCP, strlen(responseTCP), 0) < 0)
                {
                    exitError("Error sending response\n");
                }
                
                if (strcmp(responseTCP, "BYE\n") == 0)
                {
                    close(sockNew);
                    shutdown(sockNew, SHUT_RDWR);
                    break;
                }
            }
            else
            {
                close(sockNew);
                shutdown(sockNew, SHUT_RDWR);
                sockNew = 0;
            }
        }
    }
}