#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>

#define DEF_BAUD 115200
#define PRINT_BLOCK


HANDLE hCom;


uint8_t portOpen();


int main()
{
  char inBuffer[20], *tail;
  uint32_t numOfBytes;

  while(true)
  {
    uint8_t portReturn;
    do portReturn=portOpen();
    while(portReturn>1);
    if(portReturn==1) return 0;

    while(true)
    {
      printf("\nNumber of bytes to TX (x for new comm)? ");
      scanf("%s", inBuffer);
      if(inBuffer[0]=='x') break;
      do numOfBytes=strtol(inBuffer, &tail, 0);
      while(numOfBytes==0||numOfBytes>0x10000);
      uint8_t *byteBlock=malloc(numOfBytes);
      for(uint16_t byteCnt=0; byteCnt<=numOfBytes; byteCnt++) byteBlock[byteCnt]=(uint8_t)byteCnt;
      DWORD rwlen;
      WriteFile(hCom, byteBlock, numOfBytes, &rwlen, 0);

#ifdef PRINT_BLOCK
      for(uint16_t byteCnt1=0; byteCnt1<=numOfBytes; byteCnt1+=0x10)
      {
        printf("0x%04x | ", byteCnt1);
        for(uint8_t byteCnt2=0; byteCnt2<0x10; byteCnt2++) printf("0x%02x ", byteBlock[byteCnt1+byteCnt2]);
        printf("\n");
      }
#endif

      free(byteBlock);
    }
    CloseHandle(hCom);
  }
  return 0;
}


uint8_t portOpen()        /// open the com port
{
  char inBuffer[20], *tail;
  uint8_t port;
  uint32_t baud;

  CloseHandle(hCom);

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
