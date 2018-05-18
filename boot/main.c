#include <inc/x86.h>
#include <inc/elf.h>

// bootloader to boot an ELF kernel image from the first IDE hard disk
// boot.S and main.c together is the bootloader. It should be stored in the first sector of the disk.
// 2nd sector onward holds the kernel image. The kernel image must be in ELF format

// boot up steps
// 1. when the CPU boots it loads the BIOS into memory and execute it
// 2. the BIOS initializes devices, sets of the interrupt routines, and reads the first sector of the boot device (hard-drive) into memory and jumps to it.
// 3. assume this bootloader is stored in the first sector of the hard drive, this code takes over...
// 4. control starts in boot.S, setting up protected mode and a stack so C code can run, then call bootmain()
// 5. bootmain() in this file takes over, reads in the kernel and jumps to it. 

#define SECTSIZE	512
#define ELFHDR	((struct Elf *) 0x10000) // scratch space

void readsect(void*, uint32_t);
void readseg(uint32_t, uint32_t, uint32_t);

void bootmain(void)
{
	struct Proghdr *ph, *eph;

	// read 1st page off disk
	readseg((uint32_t) ELFHDR, SECTSIZE * 8, 0);

	// is this a valid ELF?
	if (ELFHDR->e_magic != ELF_MAGIC)
		goto bad;

	// load each program segment (ignores ph flags)
	ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
	eph = ph + ELFHDR->e_phnum;
	for (; ph < eph; ph++)
		// p_pa is the load address and physical address of this segment
		readseg(ph->p_pa, ph->p_memsz, ph->p_offset);

	// call the entry point from the ELF header
	((void (*)(void)) (ELFHDR->e_entry))();

bad:
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8E00);
	while(1)
		; // do nothing
}

// read 'count' bytes at 'offset' from kernel into physical address 'pa'
void readseg(uint32_t pa, uint32_t count, uint32_t offset)
{
	uint32_t end_pa;

	end_pa = pa + count;

	// round down to sector boundary
	pa &= ~(SECTSIZE - 1);

	// translate from bytes to sectors, and kernel starts at sector 1
	offset = (offset / SECTSIZE) + 1;

	while (pa < end_pa) {
		readsect((uint8_t *) pa, offset);
		pa += SECTSIZE;
		offset++;
	}
}

void waitdisk(void)
{
	// wait for disk ready
	while((inb(0x1F7) & 0xC0) != 0x40)
		; // do nothing
}

void readsect(void *dst, uint32_t offset)
{
	// wait for disk to be ready
	waitdisk();

	outb(0x1F2, 1); // count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20); // cmd 0x20, read sectors

	// wait for disk to be ready
	waitdisk();

	// read a sector
	insl(0x1F0, dst, SECTSIZE / 4);
}