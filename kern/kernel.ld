/* simple linker script for the JOS kernel.
     see the GNU ld 'info' manual to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	/* link the kernel at this address: "." means the current address */
	.= 0xF0100000;

	/* AT(...) gives the load address of this section, which tells the boot loader where to load the kernel in physical memory */
	.text : AT(0x100000) {
		*(.text .stub .text. * .gnu.linkonce.t.*)
	}

	/ * define the 'etext' symbol to this value */
	PROVIDE(etext = .);

	.rodata : {
		*(.rodata .rodata. * .gnu.linkeonce.r.*)
	}

	/* include debugging information in kernel memory */
	.stab : {
		PROVIDE(__STAB_BEGIN__=.);
		*(.stab);
		PROVIDE(_STAB_END__=.);
		/* force the linker to allocate space for this section */
		BYTE(0)
	}

	.stabstr : {
		PROVIDE(__STABSTR_BEGIN__=.);
		*(.stabstr);
		PROVIDE(__STABSTR_END__=.);
		/* force the linker to allocate space for this section */
		BYTE(0)
	}

	/* adjust the address for the data segment to the next page */
	.= ALIGN(0x1000);

	/* the data segment */
	.data : {
		*(.data)
	}

	PROVIDE(edata = .);

	.bss : {
		*(.bss)
	}

	PROVIDE(end = .);

	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
} 