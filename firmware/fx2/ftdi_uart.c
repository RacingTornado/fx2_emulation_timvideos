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
	EP1INBC = bytes_waiting_for_xmit + 2;

	bytes_waiting_for_xmit = 0;
}


static void putchar(char c)
{
   xdata unsigned char *dest=EP1INBUF + bytes_waiting_for_xmit + 2;

   // Wait (if needed) for EP1INBUF ready to accept data
   while (EP1INCS & 0x02);

   *dest = c;

   if (++bytes_waiting_for_xmit >= 62) ProcessXmitData();

   // Set Timer 0 if it isn't set and we've got data ready to go
   if (bytes_waiting_for_xmit && !(TCON & 0x10)) {
      TH0 = MSB(0xffff - latency_us);
      TL0 = LSB(0xffff - latency_us);
      TCON |= 0x10;
   }
}

static void ProcessRecvData(void)
{
	xdata const unsigned char *src=EP1OUTBUF;
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
   int i;
   int interface;

   // Errors are signaled by stalling endpoint 0.

   switch(SETUPDAT[0] & SETUP_MASK) {

   case SETUP_STANDARD_REQUEST:
      switch(SETUPDAT[1])
	 {
	 case SC_GET_DESCRIPTOR:
	    switch(SETUPDAT[3])
	       {
	       case GD_DEVICE:
		  SUDPTRH = MSB(&myDeviceDscr);
		  SUDPTRL = LSB(&myDeviceDscr);
		  break;
	       case GD_DEVICE_QUALIFIER:
		  SUDPTRH = MSB(&myDeviceQualDscr);
		  SUDPTRL = LSB(&myDeviceQualDscr);
		  break;
	       case GD_CONFIGURATION:
		  myConfigDscr.type = CONFIG_DSCR;
		  if (USBCS & bmHSM) {
		     // High speed parameters
		     myInEndpntDscr.mp = 64;
		     myOutEndpntDscr.mp = 64;
		  } else {
		     // Full speed parameters
		     myInEndpntDscr.mp = 64;
		     myOutEndpntDscr.mp = 64;
		  }
		  SUDPTRH = MSB(&myConfigDscr);
		  SUDPTRL = LSB(&myConfigDscr);
		  break;
	       case GD_OTHER_SPEED_CONFIGURATION:
		  myConfigDscr.type = OTHERSPEED_DSCR;
		  if (USBCS & bmHSM) {
		     // We're in high speed mode, this is the Other
		     // descriptor, so these are full speed parameters
		     myInEndpntDscr.mp = 64;
		     myOutEndpntDscr.mp = 64;
		  } else {
		     // We're in full speed mode, this is the Other
		     // descriptor, so these are high speed parameters
		     myInEndpntDscr.mp = 64;
		     myOutEndpntDscr.mp = 64;
		  }
		  SUDPTRH = MSB(&myConfigDscr);
		  SUDPTRL = LSB(&myConfigDscr);
		  break;
	       case GD_STRING:
		  if (SETUPDAT[2] >= count_array_size((void **) USB_strings)) {
		     EZUSB_STALL_EP0();
		  } else {
		     for (i=0; i<31; i++) {
			if (USB_strings[SETUPDAT[2]][i] == '\0') break;
			EP0BUF[2*i+2] = USB_strings[SETUPDAT[2]][i];
			EP0BUF[2*i+3] = '\0';
		     }
		     EP0BUF[0] = 2*i+2;
		     EP0BUF[1] = STRING_DSCR;
		     SYNCDELAY; EP0BCH = 0;
		     SYNCDELAY; EP0BCL = 2*i+2;
		  }
		  break;
	       default:            // Invalid request
		  EZUSB_STALL_EP0();
	       }
	    break;
	 case SC_GET_INTERFACE:
	    interface = SETUPDAT[4];
	    if (interface < NUM_INTERFACES) {
	       EP0BUF[0] = InterfaceSetting[interface];
	       EP0BCH = 0;
	       EP0BCL = 1;
	    }
	    break;
	 case SC_SET_INTERFACE:
	    interface = SETUPDAT[4];
	    if (interface < NUM_INTERFACES) {
	       InterfaceSetting[interface] = SETUPDAT[2];
	    }
	    break;
	 case SC_SET_CONFIGURATION:
	    Configuration = SETUPDAT[2];
	    break;
	 case SC_GET_CONFIGURATION:
	    EP0BUF[0] = Configuration;
	    EP0BCH = 0;
	    EP0BCL = 1;
	    break;
	 case SC_GET_STATUS:
	    switch(SETUPDAT[0])
	       {
	       case GS_DEVICE:
		  EP0BUF[0] = ((BYTE)Rwuen << 1) | (BYTE)Selfpwr;
		  EP0BUF[1] = 0;
		  EP0BCH = 0;
		  EP0BCL = 2;
		  break;
	       case GS_INTERFACE:
		  EP0BUF[0] = 0;
		  EP0BUF[1] = 0;
		  EP0BCH = 0;
		  EP0BCL = 2;
		  break;
	       case GS_ENDPOINT:
		  EP0BUF[0] = *(BYTE xdata *) epcs(SETUPDAT[4]) & bmEPSTALL;
		  EP0BUF[1] = 0;
		  EP0BCH = 0;
		  EP0BCL = 2;
		  break;
	       default:            // Invalid Command
		  EZUSB_STALL_EP0();
	       }
	    break;
	 case SC_CLEAR_FEATURE:
	    switch(SETUPDAT[0])
	       {
	       case FT_DEVICE:
		  if(SETUPDAT[2] == 1)
		     Rwuen = FALSE;       // Disable Remote Wakeup
		  else
		     EZUSB_STALL_EP0();
		  break;
	       case FT_ENDPOINT:
		  if(SETUPDAT[2] == 0)
		     {
			*(BYTE xdata *) epcs(SETUPDAT[4]) &= ~bmEPSTALL;
			EZUSB_RESET_DATA_TOGGLE( SETUPDAT[4] );
		     }
		  else
		     EZUSB_STALL_EP0();
		  break;
	       }
	    break;
	 case SC_SET_FEATURE:
	    switch(SETUPDAT[0])
	       {
	       case FT_DEVICE:
		  if((SETUPDAT[2] == 1) && Rwuen_allowed)
		     Rwuen = TRUE;      // Enable Remote Wakeup
		  else if(SETUPDAT[2] == 2)
		     // Set Feature Test Mode.  The core handles this
		     // request.  However, it is necessary for the
		     // firmware to complete the handshake phase of the
		     // control transfer before the chip will enter test
		     // mode.  It is also necessary for FX2 to be
		     // physically disconnected (D+ and D-) from the host
		     // before it will enter test mode.
		     break;
		  else
		     EZUSB_STALL_EP0();
		  break;
	       case FT_ENDPOINT:
		  *(BYTE xdata *) epcs(SETUPDAT[4]) |= bmEPSTALL;
		  break;
	       default:
		  EZUSB_STALL_EP0();
	       }
	    break;
	 default:                     // *** Invalid Command
	    EZUSB_STALL_EP0();
	    break;
	 }
      break;

   default:
      EZUSB_STALL_EP0();
      break;
   }

   // Acknowledge handshake phase of device request
   EP0CS |= bmHSNAK;
}

