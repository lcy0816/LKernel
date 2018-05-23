/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>

# Shift Right Logical
#define SRL(val, shamt)	(((val) >> (shamt)) & ~(-1 << (32 - (shamt))))

# the kernel (this code) is linked at address ~(KERNBASE + 1 Meg)
# but the bootloader loads it at address ~1 Meg

# RELOC(x) maps a symbol x from its link address to its actual location
# in the physical memory (load address)

#define RELOC(x) ((x) - KERNBASE)

#define MULTIBOOT_HEADER_MAGIC (0x1BADB002)
#define MULTIBOOT_HEADER_FLAGS (0)
#define CHECKSUM (-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS))

#entry point
.text

# the multiboot header
.align 4
.long MULTIBOOT_HEADER_MAGIC
.long MULTIBOOT_HEADER_FLAGS
.long CHECKSUM

# '_start' specifies the ELF entry point. Since we haven't set up
# virtual memory when the bootloader enters this code, we need
# the bootloader to jump to the *physical* address of the entry point
.globl	_start
_start = RELOC(entry)

.globl entry
entry:
	movw $0x1234, 0x472	# warm boot

	# the virtual memory has not been set up yet, the boot loader loaded
	# kernel at ~1MB; while the c code is lined to run at KERNBASE + 1MB.
	# here to set up a page directory to translate virtual address [KERNBASE, 
	# KERNBASE + 4MB) to physical address [0, 4MB)

	# load the physical address of entry_pgdir into cr3
	movl $(RELOC(entry_pgdir)), %eax
	movl %eax, %cr3
	# turn on paging
	movl %cr0, %eax
	orl $(CR0_PE | CR0_PG | CR0_WP), %eax
	movl %eax, %cr0

	# now paging is enabled. Jump up above KERNBASE before entering c code
	mov $relocated, %eax
	jmp *%eax
relocated:
	# clear the frame pointer register (EBP)
	# so that once we get into debugging C code
	# stack backtraces will be terminated properly
	movl $0x0, %ebp # clear frame pointer

	# set the stack pointer
	movl $(bootstacktop), %esp

	# now to C code
	call i386_init

	# in case getting here, spin
spin: jmp spin

.data
# boot stack
	.p2align PGSHIFT # force page alignment
	.globl		bootstack
bootstack:
	.space		KSTKSIZE
	.globl		bootstacktop
bootstacktop: