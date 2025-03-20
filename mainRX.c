#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>

#define DEF_BAUD 115200
#define RX_TIMEOUT 10

HANDLE hCom;
uint8_t portOpen();
int timeval_subtract(struct timespec *result, struct timespec *x, struct timespec *y);

int main()
{
    uint8_t rx = 0, portReturn;
    DWORD rwlen;
    FILE *outFile;
    struct timespec T0, T1, Tdiff = {0};

    do portReturn = portOpen();
    while (portReturn > 1);
    if (portReturn == 1) return 0;

    printf("Opening out.bin for writing...\n");
    outFile = fopen("out.bin", "wb");
    if (!outFile)
    {
        printf("Error: Could not open out.bin for writing!\n");
        return 1;
    }

    // Write a test byte to verify file operations
    uint8_t testByte = 0xAA;
    if (fwrite(&testByte, 1, 1, outFile) != 1)
    {
        printf("Error: fwrite test failed!\n");
        fclose(outFile);
        return 1;
    }
    fflush(outFile);
    printf("Test write successful, proceeding with UART data capture.\n");

    // Flush input buffer
    do ReadFile(hCom, &rx, 1, &rwlen, 0);
    while (rwlen != 0);
    printf("UART buffer flushed.\n");
    uint32_t totalBytes = 0;

    while (Tdiff.tv_sec < RX_TIMEOUT)
    {
        clock_gettime(CLOCK_MONOTONIC, &T0);
    
        do
        {
            clock_gettime(CLOCK_MONOTONIC, &T1);
            timeval_subtract(&Tdiff, &T1, &T0);
    
            if (!ReadFile(hCom, &rx, 1, &rwlen, 0))
            {
                printf("ReadFile failed with error %lu\n", GetLastError());
                break;
            }
    
            if (rwlen > 0)
            {
                totalBytes += rwlen;
                printf("Read %lu byte(s): 0x%02X (Total: %u bytes)\n", rwlen, rx, totalBytes);
    
                if (fwrite(&rx, 1, 1, outFile) != 1)
                {
                    printf("Error: fwrite failed!\n");
                    break;
                }
                fflush(outFile);
            }
            else
            {
                printf("No data read\n");
            }
    
        } while (rwlen == 0 && Tdiff.tv_sec < RX_TIMEOUT);
    }
    
    printf("Total bytes received: %u\n", totalBytes);
    
    
    fclose(outFile);
    printf("File closed. Check out.bin\n");
    return 0;
}

uint8_t portOpen()
{
    char inBuffer[20], *tail;
    uint8_t port;
    uint32_t baud;

    printf("\nCOM port? ");
    scanf("%s", inBuffer);
    if (inBuffer[0] == 'x') return 1;
    port = strtol(inBuffer, &tail, 0);

    sprintf(inBuffer, "\\\\.\\COM%d", port);
    hCom = CreateFile(inBuffer, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (hCom == INVALID_HANDLE_VALUE)
    {
        printf("Error: COM%d is not available.\n", port);
        return 2;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hCom, &dcbSerialParams))
    {
        printf("Unable to get the state of serial port\n");
        return 3;
    }

    printf("\nBaud rate? ");
    scanf("%s", inBuffer);
    baud = (inBuffer[0] == 'x') ? DEF_BAUD : strtol(inBuffer, &tail, 0);

    dcbSerialParams.BaudRate = baud;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hCom, &dcbSerialParams))
    {
        printf("Unable to set serial port settings\n");
        return 4;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 500;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    if (!SetCommTimeouts(hCom, &timeouts))
    {
        printf("Error setting Serial Port timeouts\n");
        return 5;
    }
    printf("COM%d opened successfully @ %i baud\n", port, baud);
    return 0;
}

int timeval_subtract(struct timespec *result, struct timespec *x, struct timespec *y)
{
    if (x->tv_nsec < y->tv_nsec)
    {
        int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
        y->tv_nsec -= 1000000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_nsec - y->tv_nsec > 1000000000)
    {
        int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
        y->tv_nsec += 1000000000 * nsec;
        y->tv_sec -= nsec;
    }
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_nsec = x->tv_nsec - y->tv_nsec;
    return x->tv_sec < y->tv_sec;
}
