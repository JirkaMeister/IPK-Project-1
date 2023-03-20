# IPK Project 1: IPK Calculator Protocol

## Task

Your task is to implement a client for the IPK Calculator Protocol [1].
The client should be able to communicate with any server using IPKCP, not just the provided reference implementation.

The client is started using:
`ipkcpc -h <host> -p <port> -m <mode>`
where `host` is the IPv4 address of the server, `port` the server port, and `mode` either `tcp` or `udp` (e.g., `ipkcpc -h 1.2.3.4 -p 2023 -m udp`).

The client will use the appropriate protocol variant depending on the selected mode (binary for UDP, textual for TCP).
It will read a list of commands from the standard input, one per line, and print the received responses on the standard output.
Responses should be formatted according to the examples below.
Any (client) errors should be printed to the standard **error output**.

The client should react to the *Interrupt* signal (`C-c`) by gracefully exiting, cleanly terminating the connection as appropriate for the selected mode.

### Input/Output Examples

#### Binary

Input:
```
(+ 1 2)
(a b)
```

Output:
```
OK:3
ERR:<error message>
```

#### Textual

Input:
```
HELLO
SOLVE (+ 1 2)
BYE
```

Output:
```
HELLO
RESULT 3
BYE
```

## Submission

The submission should follow all the requirements described in the general README.
The project should build a single executable called `ipkcpc` in the project root using the default `make` target.
This executable need not be self-contained, and can be implemented as a wrapper/launcher, but it should conform to the CLI interface described above.

## References

[1] [IPK Calculator Protocol](./Protocol.md)
