// See COPYRIGHT for copyright information

#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/kbdreg.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/console.h>

static void cons_intr(int (*proc)(void));
static void cons_putc(int c);

static void delay(void) // delay routine necessary for historical design
{
	inb(0x84);
	inb(0x84);
	inb(0x84);
	inb(0x84);
}

// Serial I/O code
#define COM1			0x3F8

#define COM_RX		0 // in: receive buffer (DLAB = 0)
#define COM_TX		0 // out: transmit buffer (DLAB = 0)
#define COM_DLL		0 // out: divisor latch low (DLAB = 1)
#define COM_DLM		1 // out: divisor latch high (DLAB = 1)
#define COM_IER		1 // out: interrupt enable register
#define COM_IER_RDI		0x01 // enable receiver data interrupt
#define COM_IIR		2 // inL interrupt id register
#define COM_FCR		2 // out: FIFO control register
#define COM_LCR		3 // out: line control register
#define COM_LCR_DLAB	0x80 // divisor latch access bit
#define COM_LCR_WLEN8	0x03 // wordlength: 8 bits
#define COM_MCR		4 // out: modem control register
#define COM_MCR_RTS	0x02 // RTS complement
#define COM_MCR_DTR	0x01 // DTR complement
#define COM_MCR_OUT2	0x08 // out2 complement
#define COM_LSR		5 // in: line status register
#define COM_LSR_DATA	0x01 // data available
#define COM_LSR_TXRDY	0x20 // transmit buffer available
#define COM_LSR_TSRE	0x40 // transmitter off

static bool serial_exists;

static int serial_proc_data(void)
{
	if (!(inb(COM1 + COM_LSR) & COM_LSR_DATA))
		return -1;
	return inb(COM1 + COM_RX);
}

void serial_intr(void)
{
	if (serial_exists)
		cons_intr(serial_proc_data);
}

static void serial_putc(int c)
{
	int i;

	for (i = 0;
	      !(inb(COM1 + COM_LSR) & COM_LSR_TXRDY) && i < 12800;
	      i++)
		delay();
	outb(COM1 + COM_TX, c);
}

static void serial_init(void)
{
	// turn off FIFO
	outb(COM1 + COM_FCR, 0);

	// set speed, requires DLAB latch
	outb(COM1 + COM_LCR, COM_LCR_DLAB);
	outb(COM1 + COM_DLL, (uint8_t)(115200 /9600));
	outb(COM1 + COM_DLM, 0);

	// 8 data bits, 1 stop bit, parity off, turn off DLAB latch
	outb(COM1 + COM_LCR, COM_LCR_WLEN8 & ~COM_LCR_DLAB);

	// no modem controls
	outb(COM1 + COM_MCR, 0);
	// enable rcv interrupts
	outb(COM1 + COM_IER, COM_IER_RDI);

	// clear any preesiting overrun indications and interrupts
	// serial port doesn't exist if COM_LSR returns 0xFF
	serial_exists = (inb(COM1 + COM_LSR) != 0xFF);
	(void) inb(COM1 + COM_IIR);
	(void) inb(COM1 + COM_RX);
}

// parallel port output code
static void lpt_putc(int c)
{
	int i;
	for (i = 0; !(inb(0x378 + 1) & 0x80) && i < 12800; i++)
		delay();
	outb(0x378 + 0, c);
	outb(0x378 + 2, 0x08 | 0x04 | 0x01);
	outb(0x378 + 2, 0x08);
}

