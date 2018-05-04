#include <inc/mmu.h>

# start the CPU: switch to 32-bit protected mode, jump into C
# this BIOS loads this code from the first sector of the hard disk into 
# memory at physical address 0x7c00 and starts in real mode with %cs=0 %ip=7c00

.set PROT_MODE_CSEG, 0x8		# kernel code segment selector
.set PROT_MODE_DSEG, 0x10		# kernel data segment selector
.set CR0_PE_ON, 0x1 			# protected mode enable flag

.global start
start:
  .code16				# assembler for 16-bit mode
  cli					# disable interrupts
  cld					# string operations increment

  # set up the important data segment registers (DS, ES, SS)
  xorw	%ax, %ax			# segment data clear
  movw	%ax, %ds 			# set data segment
  movw	%ax, %es 			# set extra segment
  movw	%ax, %ss 			# set stack segment

  # enable A20:
seta20.1:
  inb	$0x64, %al 			# wait for not busy
  testb	$0x2, %al
  jnz	seta20.1

  movb	$0xd1, %al 			# write 0xd1 to port 0x64
  outb	%al, $0x64

seta20.2:
  inb	$0x64, %al 			# wait for not busy
  testb	$0x2, %al
  jnz	seta20.2

  movb	$0xdf, %al 			# write 0xdf to port 0x60
  outb	%al, $0x60

  # switch from real to protected mode, using a bootstrap GDT and segment translation
  # that makes virtual addresses identical to physical addresses
  lgdt	gdtdesc
  movl	%cr0, %eax 
  orl	$CR0_PE_ON, %eax
  movl	%eax, %cr0

  # jump to next instruction, but in 32-bit code segment
  # switches process into 32-bit mode
  ljmp	$PROT_MODE_CSEG, $protcseg 

  .code32				# assemble for 32-bit mode
protcseg:
  # set up the protected-mode data segment registers
  movw	$PROT_MODE_DSEG, %ax	# data segment selector
  movw	%ax, %ds 			# data segment
  movw	%ax, %es 			# extra segment
  movw	%ax, %fs 			# fs
  movw	%ax, %gs 			# gs
  movw	%ax, %ss 			# stack segment

  # set up the stack pointer and call into C
  movl	$start, %esp
  call bootmain

  # if bootmain returns (it shouldn't), loop
spin:
  jmp spin

# bootstrap GDT
.p2align 2				# force 4 byte alignment
gdt:
  SEG_NULL				# null seg
  SEG(STA_X | STA_R, 0x0, 0xffffffff)	# code seg
  SEG(STA_W, 0x0, 0xffffffff)		# data seg

gdtdesc:
  .word	0x17				# sizeof(gdt) - 1
  .long	gdt 				# address gdt