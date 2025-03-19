Once port and baud rate are entered the program will wait up to 5 seconds for received data.  The 5 second timeout will begin again after the received data pauses.  The program sends the ouput to "out.bin".

libwinpthread-1.dll needs to be in the same subdirectory as radioRXtest.exe

The timeout of 5 seconds can be changed by editing the '5' in the line:

#define RX_TIMEOUT 5