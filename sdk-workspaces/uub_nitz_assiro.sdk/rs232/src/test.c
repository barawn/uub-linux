
#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <stdbool.h>


#define SOH 0x01         // Start of Header
#define ETB 0x17         // End of transmission block
#define ACK 0x06         // Acknowledged
#define NAK 0x15         // Not Acknowledged
#define GS  0x1D         // Group separator
#define FP_WATT1_PROG   0x13
#define FP_CARDIO  0x2F

 int main(){
	 printf("try to open serial port...");
/* Open File Descriptor */
int RS = open( "/dev/ttyPS0", O_RDWR| O_NONBLOCK | O_NDELAY );

/* Error Handling */
if ( RS < 0 )
{
printf("Error beim oeffnen");
}

  // Configure Port
struct termios tty;
struct termios tty_old;
memset (&tty, 0, sizeof tty);

// Error Handling
if ( tcgetattr ( RS, &tty ) != 0 ) {
   printf("error beim tcgetattr");
}

// Save old tty parameters
tty_old = tty;

// Set Baud Rate of 9600 Bps
cfsetospeed (&tty, (speed_t)B9600);
cfsetispeed (&tty, (speed_t)B9600);

// 8 Bits, No Parity and 1 Stop bit settings
tty.c_cflag     &=  ~PARENB;            // No Parity
tty.c_cflag     &=  ~CSTOPB;            // 1 Stop Bit
tty.c_cflag     &=  ~CSIZE;
tty.c_cflag     |=  CS8;                // 8 Bits

tty.c_cflag     &=  ~CRTSCTS;           // no flow control
tty.c_cc[VMIN]   =  1;                  // read doesn't block
tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
tty.c_cflag     |=  (CLOCAL | CREAD );    // turn on READ & ignore ctrl lines

// Make raw
cfmakeraw(&tty);


// Flush Port, then apply attributes
tcflush( RS, TCIFLUSH );
if ( tcsetattr ( RS, TCSANOW, &tty ) != 0)
{
printf("from tcsetattr");
}

// WRITE
    // Set the Ergometer language.
char cmd[255] = {0x60,0x01};  // Ergometer Cockpit Adresse

int n_written = write( RS, cmd, sizeof(cmd));
      printf("%s\n",cmd);
    // Allocate memory for read buffer


 int bb = 0;
 int i,x = 0;
 bool cmdOK=false;
 char marr[10];

  do
   {

    if((bb = read(RS, &marr[x], sizeof( marr )-x )) > 0)
   {
     for (i=x;i<x+bb;i++)
     {

      //printf("0x%02x",marr[i]);
      if ((marr[i]==ACK)||(marr[i]==NAK)) cmdOK=true;
    }
  x+=bb;
  //printf("\n");
  //printf("x=%d\n",x);
    }

   }while(!cmdOK);

    printf("-Ausgabe---------------------\n");
    for (i=0; i<x; i++)
    {
      printf("0x%02x",marr[i]);
    }
    close(RS);

    // ttyUSB0 auf alten Wert setzen
    tcflush(RS,TCIFLUSH);
    if (tcsetattr(RS, TCSANOW, &tty) != 0) return(-1);


    exit(0);
    return 0;
    }
