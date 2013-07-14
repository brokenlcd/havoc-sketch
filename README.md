HAVOC
=====

Havoc is an open-source hardware random number generator that uses a pair of
avalanche noise-generated random bit generators.

This sketch will communicate with the host over serial. By default, it uses a
9600 baud rate. It sends messages to the host in the form <code> SP <data>.
There are two types of messages currently in use:

        1. Message code 0: PRNG fault
        2. Message code 1: random byte output

For example, a random byte output message might look like this in a hex editor

        00000000  01 20 0e 0a                                |. ..|
        00000004

The first byte is the message code (0x01), the second byte is a
space separator (0x20), the third byte is the random byte being
output by the PRNG, and a newline separator. All messages have a
newline at the end of them for easier processing on the host side.

A fault code might look like

        00000000  00 20 52 42 47 30 20 66  61 75 6c 74 0a    |. RBG0 fault.|
        0000000d

Once again, the first byte is the message code (0x00), and the
second byte is the space separator (0x20). However, the message
consists of ASCII text describing the fault.

A host program should read the serial input, scanning for newlines. When
it encounters a random byte output, it can use that byte as random data.

Currently, the following areas are potential faults:

        * An RBG returns an unexpected analog value.
