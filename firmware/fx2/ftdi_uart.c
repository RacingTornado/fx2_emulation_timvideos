

#include <fx2macros.h>
#include <fx2ints.h>
#include <autovector.h>
#include <delay.h>
#include <setupdat.h>

#include "cdc.h"

#define SYNCDELAY SYNCDELAY4

#define FTDI_RS0_CTS    (1 << 4)
#define FTDI_RS0_DSR    (1 << 5)
#define FTDI_RS0_RI     (1 << 6)
#define FTDI_RS0_RLSD   (1 << 7)

#define FTDI_RS_DR  1
#define FTDI_RS_OE (1<<1)
#define FTDI_RS_PE (1<<2)
#define FTDI_RS_FE (1<<3)
#define FTDI_RS_BI (1<<4)
#define FTDI_RS_THRE (1<<5)
#define FTDI_RS_TEMT (1<<6)
#define FTDI_RS_FIFO  (1<<7)




//There are different endpoints on the FX2. However the CPU can access the endpoint 0 or endpoint 1 which is
//smaller. In this case, the endpoint needs to be configured by setting the EPIOUTCFG, and EP1INCFG
//registers . For endpoint configuration, refer Table 8.2

static void ProcessXmitData(void)
{
	// reset Timer 0
	TCON &= ~0x30;

	// Lead in two bytes in the returned data (modem status and
	// line status).
	EP1INBUF[0] = FTDI_RS0_CTS | FTDI_RS0_DSR | 1;
	EP1INBUF[1] = FTDI_RS_DR;

	// Send the packet.
	SYNCDELAY;
	//EP1INBC = bytes_waiting_for_xmit + 2;

	//bytes_waiting_for_xmit = 0;
}


static void putchar(char c)
{

}

static void ProcessRecvData(void)
{
	__xdata const unsigned char *src=EP1OUTBUF;
	unsigned int len = EP1OUTBC;
	unsigned int i;

	// Skip the first byte in the received data (it's a port
	// identifier and length).
	src++; len--;

	for(i=0; i<len; i++,src++)
	   {
	      if(*src>='a' && *src<='z')
		 {  putchar(*src-'a'+'A');  }
	      else
		 {  putchar(*src);  }
	   }

	EP1OUTBC=0xff; // re-arm endpoint 1 for OUT (host->device) transfers
	SYNCDELAY;
}





    /*
     * USB setup packet descriptors
     *                           ____________________________________________________________
     *                          | Byte  | Field            | Meaning                          |
     *                          |_______|__________________|__________________________________|
                                | 0     | RequestType      |   Direction, and Recepient       |
     *                          | 1     | Request          |   The actaul request             |
     *                          | 2     | wValueL          |   16 bit value                   |
     *                          | 3     | wValueH          |   16 bit value                   |
                                | 4     | wIndexL          |   16 bit value                   |
                                | 5     | wIndexH          |   16 bit value                   |
                                | 6     | wLengthL         |   Bytes to transfer              |
                                | 7     | wLengthH         |   Bytes to transfer              |
     *                          |_______|__________________|__________________________________|
     *
     *
     */

//See table 2.2 for more information
static void SetupCommand(void)
{

}









//Check what FX2 configs are changed between this and original firmware

static void Initialize(void)
{
	// Note that increasing the clock speed to 24 or 48 MHz would
	// affect our timer calculations below.  I use 12 MHz because
	// this lets me use the smallest numbers for our counter (i.e,
	// 40000 for the default 40 ms latency); the counter is only
	// sixteen bits.

	IFCONFIG=0xc0;  // Internal IFCLK, 48MHz; A,B as normal ports.
	SYNCDELAY;
	//Chip Revision Control Register
	REVCTL=0x03;  // See TRM...
	SYNCDELAY;

	// Endpoint configuration - everything disabled except
	// bidirectional transfers on EP1.

	EP1OUTCFG=0xa0;
	EP1INCFG=0xa0;
	EP2CFG=0;
	EP4CFG=0;
	EP6CFG=0;
	EP8CFG=0;

	SYNCDELAY;
	EP1OUTBC=0xff; // Arm endpoint 1 for OUT (host->device) transfers
	//See section 8.6.1.5

	// Setup Data Pointer - AUTO mode
	//
	// In this mode, there are two ways to send data on EP0.  You
	// can either copy the data into EP0BUF and write the packet
	// length into EP0BCH/L to start the transfer, or you can
	// write the (word-aligned) address of a USB descriptor into
	// SUDPTRH/L; the length is computed automatically.
	SUDPTRCTL = 1;
    //See 15.12.27
	// Enable USB interrupt
	IE = 0x80;
	EIE = 0x01;

	// Enable SUDAV (setup data available) interrupt
	USBIE = 0x01;
}


void config_uart()
{
   Initialize();


}

