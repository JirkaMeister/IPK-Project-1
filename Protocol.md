# IPK Calculator Protocol

This document describes the IPK Calculator Protocol (IPKCP) and its variants.

IPKCP is an application protocol for communicating with an Arithmetic as a Service (AaaS) provider.
Clients send an expression to be evaluated, and, assuming the expression is well-formed, receive a response containing the resulting value.
IPKCP uses a prefix notation with generalized n-ary operators.

Using ABNF notation [1], a well-formed value is formally defined as:
```
operator = "+" / "-" / "*" / "/"
expr = "(" operator 2*(SP expr) ")" / 1*DIGIT
query = "(" operator 2*(SP expr) ")"
```

A server implementation MAY define additional custom operators, but it MUST support at least these four.

IPKCP has a binary and a textual variant, described below.

## Binary Variant

The binary variant is stateless and operates over UDP.
Every message begins with an opcode specifying the type of the message and contains the payload length in bytes.
The payload data is encoded as an ASCII string.

The request data contains a well-formed expression as defined above.
A response with the *OK* status contains the computed numerical value; *Error* responses contain an error message.

### Opcodes

| Opcode   | Value |
|:---------|:------|
| Request  | 0     |
| Response | 1     |

### Request Message Format

```
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +---------------+---------------+-------------------------------+
 |     Opcode    |Payload Length |          Payload Data         |
 |      (8)      |      (8)      |                               |
 +---------------+---------------+ - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
```

### Status Codes

| Status | Value |
|:-------|:------|
| OK     | 0     |
| Error  | 1     |

### Response Message Format

```
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +---------------+---------------+---------------+---------------+
 |     Opcode    |  Status Code  |Payload Length | Payload Data  |
 |      (8)      |      (8)      |      (8)      |               |
 +---------------+---------------+---------------+ - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
```

## Textual Variant

The textual variant is stateful and operates over TCP.
Communication begins in the *Init* state with a *hello* greeting from the client, to which the server responds with *hello* if accepting requests, transitioning to the *Established* state.
Afterwards, the client CAN send queries with the *solve* command and should receive the computed result.
Any number of *solve* queries can be sent in this state, including none.
When the client wishes to terminate the connection, it MUST send a *bye* command.
The server will reply with *bye* before terminating its part of the connection.

In case of a malformed, illegal, or otherwise unexpected message, the server will respond with *bye* and close the connection.

# Message Format

```
hello = "HELLO" LF
solve = "SOLVE" SP query LF
result = "RESULT" SP 1*DIGIT LF
bye = "BYE" LF
```

# References

[1] https://www.rfc-editor.org/info/std68
