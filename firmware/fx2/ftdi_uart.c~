

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
	unsigned int len = EP1OUTBC;//Tells you how many bytes
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
	//Activate and set EP1 end point
    //In cypress TRM, according to 8.4 , there needs to be a SYNCDELAY after every configuration. Edited
	EP1OUTCFG=0xa0;
    SYNCDELAY;
	EP1INCFG=0xa0;
    SYNCDELAY;
	EP2CFG=0;
    SYNCDELAY;
	EP4CFG=0;
    SYNCDELAY;
	EP6CFG=0;
    SYNCDELAY;
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
	//Set the max packet size to 64 on End Point 0 also
	//The method of writing depends on the SPDAuto bit .
	//SPDAUTO = 0 means manual mode where firmware writes the block length into
	//the EP0BCH:L register
	//Here the auto mode is used, which can be used for sending speed descriptors only.
	//No explicit loding of the EP0BCH:L is needed in this case.
	//The SUDPTRCTl is the SPDAUTO bit, table 8-7
    //See 15.12.27
	SUDPTRCTL = 1;

	// Enable USB interrupt
	IE = 0x80;
	EIE = 0x01;

	// Enable SUDAV (setup data available) interrupt
	//See 15.7.7 for more details
	USBIE = 0x01;
}


void config_uart()
{
    //See section 3.9
    //First set bit.3 in USBCS to 1, DISCON bit
    USBCS |= bmDISCON;
    //Do all your initialization. Clock frequency and UART baud rate generator
	Initialize();
	//Set bit.1 in USBCS, firmware handles all device requests
	USBCS |= bmRENUM;
	//Retain USBCS setting and toggle only the DISCON bit
	USBCS &= ~(bmDISCON);

	// Configure Timer 0 (but leave it unset)
	// mode 1 (16-bit); ungated; divide system clock by 12
	//This is the CKCON bit
	//This timer increments every 12 CLKOUT cycles




	//The other 3 LSB bits are for memeory access stretch
	/*
    ╔═════════╤══════════════════════════════════════════════════════════════╗
    ║ Bit     │ Function                                                     ║
    ╠═════════╪══════════════════════════════════════════════════════════════╣
    ║ CKCON.5 │ T2M                                                          ║
    ║         │ - Timer 2 clock select. When T2M = 0, Timer 2 uses CLK-      ║
    ║         │ OUT/12 (for compatibility with standard 8051); when T2M = 1, ║
    ║         │ Timer 2 uses CLKOUT/4. This bit has no effect when Timer 2   ║
    ║         │ is configured for baud rate generation.                      ║
    ╟─────────┼──────────────────────────────────────────────────────────────╢
    ║ CKCON.4 │ T1M                                                          ║
    ║         │ - Timer 1 clock select. When T1M = 0, Timer 1 uses CLK-      ║
    ║         │ OUT/12 (for compatibility with standard 8051); when T1M = 1, ║
    ║         │ Timer 1 uses CLKOUT/4.                                       ║
    ╟─────────┼──────────────────────────────────────────────────────────────╢
    ║ CKCON.3 │ T0M                                                          ║
    ║         │ - Timer 0 clock select. When T0M = 0, Timer 0 uses CLK-      ║
    ║         │ OUT/12 (for compatibility with standard 8051); when T0M = 1, ║
    ║         │ Timer 0 uses CLKOUT/4                                        ║
    ╚═════════╧══════════════════════════════════════════════════════════════╝
    */



    //I dont know if this can be bit address since in fx2regs.h , there doesnt seem to be split up
	CKCON &= ~(0x08);
	/*
    ╔════════╤═══════════════════════════════════════════════════════════════════════════╗
    ║ Bit    │ Function                                                                  ║
    ╠════════╪═══════════════════════════════════════════════════════════════════════════╣
    ║ TMOD.7 │ GATE1                                                                     ║
    ║        │ - Timer 1 gate control. When GATE1 = 1, Timer 1 will clock only when INT1 ║
    ║        │ = 1 and TR1 (TCON.6) = 1. When                                            ║
    ║        │ GATE1 = 0, Timer 1 will clock only when TR                                ║
    ║        │ 1 = 1, regardless of the state of INT1                                    ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.6 │ C/T                                                                       ║
    ║        │ 1                                                                         ║
    ║        │ - Counter/Timer select. When C/T                                          ║
    ║        │ 1 = 0, Timer 1 is clocked by CLKOUT/4                                     ║
    ║        │ or CLKOUT/12, depending on the state of                                   ║
    ║        │ T1M (CKCON.4). When C/T                                                   ║
    ║        │ 1 = 1, Timer 1 is clocked by high-to-low transitions on the T1 pin.       ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.5 │ M1                                                                        ║
    ║        │ - Timer 1 mode select bit 1.                                              ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.4 │ M0                                                                        ║
    ║        │ - Timer 1 mode select bit 0.                                              ║
    ║        │ Mode                                                                      ║
    ║        │ 0       0     Mode 0: 13 bit counter                                      ║
    ║        │ 0       1     Mode 1: 16 bit counter                                      ║
    ║        │ 1       0     Mode 2: 8 bit counter with auto-reload                      ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.3 │ GATE0                                                                     ║
    ║        │ - Timer 0 gate control, When GATE0 = 1, Timer 0 will clock only when INT0 ║
    ║        │ = 1 and TR0 (TCON.4) = 1. When                                            ║
    ║        │ GATE0 = 0, Timer 0 will clock only when TR                                ║
    ║        │ 0 = 1, regardless of the state of INT0                                    ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.2 │ C/T                                                                       ║
    ║        │ 0                                                                         ║
    ║        │ - Counter/Timer select. When C/T                                          ║
    ║        │ 0 = 0, Timer 0 is clocked by CLKOUT/4                                     ║
    ║        │ or CLKOUT/12, depending on the state of                                   ║
    ║        │ T0M (CKCON.3). When C/T                                                   ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.1 │ - Timer 0 mode select bit 1.                                              ║
    ╟────────┼───────────────────────────────────────────────────────────────────────────╢
    ║ TMOD.0 │ - Timer 0 mode select bit 0.                                              ║
    ╚════════╧═══════════════════════════════════════════════════════════════════════════╝
    */
	TMOD = 0x01;
	//TCON

	/*

    ╔════════╤══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║ Bit    │ Function                                                                                                             ║
    ╠════════╪══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
    ║ TCON.7 │ TF1                                                                                                                  ║
    ║        │ - Timer 1 overflow flag. Set to 1 when the Timer 1 count overflows; automatically cleared when the EZ-USB vectors to ║
    ║        │ the interrupt service routine.                                                                                       ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.6 │ TR1                                                                                                                  ║
    ║        │ - Timer 1 run control. 1 = Enable counting on Timer 1.                                                               ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.5 │ TF0                                                                                                                  ║
    ║        │ - Timer 0 overflow flag. Set to 1 when the Timer 0 count overflows; automatically cleared when the EZ-USB vectors to ║
    ║        │ the interrupt service routine.                                                                                       ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.4 │ TR0                                                                                                                  ║
    ║        │ - Timer 0 run control. 1 = Enable counting on Timer 0.                                                               ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.3 │ IE1                                                                                                                  ║
    ║        │ - Interrupt 1 edge detect. If external interrupt 1 is confi                                                          ║
    ║        │ gured to be edge-sensitive (IT1 = 1), IE1 is set when a negative                                                     ║
    ║        │ edge is detected on the INT1                                                                                         ║
    ║        │ pin and is automatically cleared when the EZ-USB vectors to the corresponding interrupt ser-                         ║
    ║        │ vice routine. In this case, IE1 can also be cleared by softwa                                                        ║
    ║        │ re. If external interrupt 1 is                                                                                       ║
    ║        │ configured to be level-sensitive                                                                                     ║
    ║        │ (IT1 = 0), IE1 is set when the INT1                                                                                  ║
    ║        │ pin is 0 and automatically cleared when the INT1                                                                     ║
    ║        │ pin is 1. In level-sensitive mode, soft-                                                                             ║
    ║        │ ware cannot write to IE1.                                                                                            ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.2 │ IT1                                                                                                                  ║
    ║        │ - Interrupt 1 type select. INT1                                                                                      ║
    ║        │ is detected on falling edge when IT1 = 1; INT1                                                                       ║
    ║        │ is detected as a low level when IT1 = 0.                                                                             ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.1 │ IE0                                                                                                                  ║
    ║        │ - Interrupt 0 edge detect. If external interrupt 0 is confi                                                          ║
    ║        │ gured to be edge-sensitive (IT0 = 1), IE0 is set when a negative                                                     ║
    ║        │ edge is detected on the INT0                                                                                         ║
    ║        │ pin and is automatically cleared when the EZ-USB vectors to the corresponding interrupt ser-                         ║
    ║        │ vice routine. In this case, IE0 can also be cleared by softwa                                                        ║
    ║        │ re. If external interrupt 0 is                                                                                       ║
    ║        │ configured to be level-sensitive                                                                                     ║
    ║        │ (IT0 = 0), IE0 is set when the INT0                                                                                  ║
    ║        │ pin is 0 and automatically cleared when the INT0                                                                     ║
    ║        │ pin is 1. In level-sensitive mode, soft-                                                                             ║
    ║        │ ware cannot write to IE0.                                                                                            ║
    ╟────────┼──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╢
    ║ TCON.0 │ IT0                                                                                                                  ║
    ║        │ - Interrupt 0 type select. INT0                                                                                      ║
    ║        │ is detected on falling edge when IT0 = 1; INT0                                                                       ║
    ║        │ is detected as a low level when IT0 = 0                                                                              ║
    ╚════════╧══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

	*/



	TCON &= ~0x30;





}

