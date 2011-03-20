#include "sq.h"


/** This is a C driver for Serial port. We use it for debugging and
 *  comunicating with the outside world (when running in VMware or
 *  like).
 *  It's pretty basic, if you want to improve it, you are welcome.
**/ 


static  unsigned char inb(unsigned short port)
{
   unsigned char ret;
	asm("in %1, %0" : "=a" (ret) : "d" ((unsigned short)port));
   return ret;
}

static  unsigned char outb(unsigned short port, unsigned char byte)
{
	asm("out %0, %1" :: "a" ((unsigned char)byte), "d" ((unsigned short)port));
}

#define PORT 0x3f8   /* COM1 */

void init_serial()
{
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   //outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 0, 0x02);    // Set divisor to 2 (lo byte) 57600 baud   
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
//   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14 (16?)-byte threshold
   outb(PORT + 2, 0x07);    // Enable FIFO, clear them, with 1-byte threshold
//   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
//   outb(PORT + 4, 0x00);    // IRQs disabled completely
}

void read_all_registers()
{
	int i;
	for (i = 0; i < 8; i++) {
		unsigned char reg = inb(PORT + i);
		//printf_pocho("Reg %d value: %x\n", i, (unsigned int)reg);
	}
}


int serial_received()
{

	// Don't ask me why, but adding this fixes everything
	// and I don't wan't to bother what the real problem is.
	static int i = 0;
	i++;
	if (i == 10000)
		read_all_registers();
		
	// until here
   return inb(PORT + 5) & 1;
}
 
unsigned char read_serial()
{
   while (serial_received() == 0);
 
   return inb(PORT);
}


int is_transmit_empty()
{
	static int count = 0;
	int res = inb(PORT + 5) & 0x20;
	if (res == 0)
		count++;
	else
		count = 0;
	
	if (count == 10000)
	{
		read_all_registers();
		count = 0;
		res = 1;
	}
	return res;
}
 
void write_serial(unsigned char a)
{
   // separate in two 4 bit sends because it doesn't seem to be able to send 8 bits at once

   while (is_transmit_empty() == 0);
   outb(PORT, a & 0x0F);
   
   while (is_transmit_empty() == 0);
   outb(PORT, (a>>4) & 0x0F);

}

void write_serial_string(char *string)
{
	while (*string != 0)
	{
		write_serial(*string);
		string++;
	}
}


void enter_debug_mode()
{
	printf_pocho("Waiting for debug commands via serial port.\n");
	printf_pocho("Write an address and I'll tell you its value.\n");
	printf_pocho("if you do $> nm SqueakNOS.kernel | grep var_name \nin bash you'll get var_name address in memory for any VM var\n");
	
	init_serial();
	
	
	while (1)
	{
		unsigned char next_address[4];
		printf_pocho("Waiting... ");
		next_address[0] = read_serial();
		//printf_pocho("Received something: %x\n", next_address[0]);
		next_address[1] = read_serial();
		next_address[2] = read_serial();
		next_address[3] = read_serial();
		
		unsigned int *array_address = (unsigned int*)next_address;
		unsigned int *value = (unsigned int*)*array_address;

		printf_pocho("Address is 0x%x, its 32bit value: %u (0x%x)\n", value, *value, *value);
	}
}
