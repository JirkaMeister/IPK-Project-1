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

int sockfd;
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


double solve(char *str, bool *success)
{
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

            double result = solve(tmpString);
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


response_t handleRequest(message_t message)
{
    response_t response;
    response.opcode = '\x01';

    if (message.opcode == '\x00')
    {
        fprintf(stderr, "Message opcode: %d\n", message.opcode);
        fprintf(stderr, "Message payload length: %d\n", message.payloadLength);
        fprintf(stderr, "Message payload: %s\n", message.payload);
        response.status = '\x00';
    }
    else
    {
        response.status = '\x01';
    }

    for(int i = 0; i < message.payloadLength; i++)
    {
        if (message.payload[i] == '\n' || message.payload[i] == '\r')
        {
            message.payload[i] = '\0';
            break;
        }
    }
    response.payloadLength = strlen(message.payload);
    bool success = true;
    double result = solve(message.payload, &success);
    fprintf(stderr, "Result: %lf\n", result);
    if (!success)
    {
        response.status = '\x01';
        strcpy(response.payload, "Could not parse the message");
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
    
    return response;
}

int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    sockfd = setupSocket();

    struct sockaddr_in server_addr = setupAdress(sockfd, SERVER);

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    struct sockaddr *client_address = (struct sockaddr *) &client_addr;

    while(true)
    {
        message_t message;
        response_t response;
        response.opcode = '\x01';

        fprintf(stderr, "Waiting for message\n");

        if (recvfrom(sockfd, &message, sizeof(message), 0, client_address, &client_addr_size) < 0)
        {
            exitError("Error receiving message\n");
        }
        else
        {
            fprintf(stderr, "Received message\n");
        }

        // Handle message
        response = handleRequest(message);

        if (sendto(sockfd, &response, sizeof(response), 0, client_address, client_addr_size) < 0)
        {
            exitError("Error sending response\n");
        }
        else
        {
            fprintf(stderr, "Sent message\n\n");
        }

    }
}