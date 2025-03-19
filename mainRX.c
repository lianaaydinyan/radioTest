#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>


#define DEF_BAUD 19200

#define RX_TIMEOUT 5


HANDLE hCom;


uint8_t portOpen();
int timeval_subtract(struct timespec *result, struct timespec *x, struct timespec *y);


int main()
{
  uint8_t rx=0, portReturn;
  DWORD	rwlen;
  FILE *outFile;
  struct timespec T0, T1, Tdiff= {0};

  do portReturn=portOpen();
  while(portReturn>1);
  if(portReturn==1) return 0;

  printf("outFile = %s\n\n", "out.bin");
  outFile=fopen("out.bin", "wb");
  if(!outFile)
  {
    printf("\n\noutFile bomb!\n\n");
    return 1;
  }

  do ReadFile(hCom, &rx, 1, &rwlen, 0);
  while(rwlen!=0);
  printf("flushed\n\n");

  while(Tdiff.tv_sec<RX_TIMEOUT)
  {
    clock_gettime(CLOCK_MONOTONIC, &T0);
    do
    {
      clock_gettime(CLOCK_MONOTONIC, &T1);
      timeval_subtract(&Tdiff, &T1, &T0);
      ReadFile(hCom, &rx, 1, &rwlen, 0);
    }
    while(rwlen==0&&Tdiff.tv_sec<RX_TIMEOUT);

    fwrite(&rx, 1, 1, outFile);
  }
  fclose(outFile);
  return 0;
}

uint8_t portOpen()        /// open the com port
{
  char inBuffer[20], *tail;
  uint8_t port;
  uint32_t baud;

  printf("\nCOM port? ");
  scanf("%s", inBuffer);
  if(inBuffer[0]=='x') return 1;
  port=strtol(inBuffer, &tail, 0);

  /*	Open COM device	*/
  sprintf(inBuffer, "\\\\.\\COM%d", port);
  hCom = CreateFile(inBuffer, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  if(hCom==INVALID_HANDLE_VALUE)
  {
    printf("\terror: COM%d is not available.\n", port);
    return 2;
  }

  DCB dcbSerialParams = {0};
  dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
  if (!GetCommState(hCom, &dcbSerialParams))
  {
    printf("Unable to get the state of serial port");
    return 3;
  }

  printf("\nBaud rate? ");
  scanf("%s", inBuffer);
  if(inBuffer[0]=='x') baud=DEF_BAUD;
  else baud=strtol(inBuffer, &tail, 0);

  dcbSerialParams.BaudRate=baud;
  dcbSerialParams.ByteSize=8;
  dcbSerialParams.StopBits=ONESTOPBIT;
  dcbSerialParams.Parity=NOPARITY;
  if(!SetCommState(hCom, &dcbSerialParams))
  {
    printf("Unable to set serial port settings\n");
    return 4;
  }

  COMMTIMEOUTS timeouts= {0};
  timeouts.ReadIntervalTimeout=50;
  timeouts.ReadTotalTimeoutConstant=50;
  timeouts.ReadTotalTimeoutMultiplier=10;
  timeouts.WriteTotalTimeoutConstant=50;
  timeouts.WriteTotalTimeoutMultiplier=10;
  if(!SetCommTimeouts(hCom, &timeouts))
  {
    printf("Error setting Serial Port timeouts property\n");
    return 5;
  }
  printf("COM%d opened successfully @ %i baud\n", port, baud);

  return 0;
}

int timeval_subtract(struct timespec *result, struct timespec *x, struct timespec *y)
{
  /* Perform the carry for the later subtraction by updating y. */
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
  /* Compute the time remaining to wait.
  tv_nsec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;
  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
