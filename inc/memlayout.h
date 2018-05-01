#ifndef JOS_INC_MEMLAYOUT_H
#define JOS_INC_MEMLAYOUT_H

// not assembler
#ifndef __ASSEMBLER__
#include <inc/types.h>
#include <inc/mmu.h>
#endif

// memory management in the OS for both kernel and user-mode software
// Global descriptor numbers
#define GD_KT		0x08 // kernel text
#define GD_KD		0x10 // kernel data
#define GD_UT		0x18 // user text
#define GD_UD 	0x20 // user data
#define GD_TSS0	0x28 // task segment selector for CPU 0

// All physical memory mapped at this address
#define KERNBASE	0xF0000000

// At IOPHYSMEM (640K) there is a 384K hole for I/O. From the kernel,
// IOPHYSMEM can be addressed at KERNBASE + IOPHYSMEM. The hole ends
// at physical address EXTPHYSMEM.
#define IOPHYSMEM	0x0A0000
#define EXTPHYSMEM	0x100000

// Kernel stack
#define KSTACKTOP	KERNBASE
#define KSTKSIZE	(8 * PGSIZE) // size of a kernel stack
#define KSTKGAP	(8 * PGSIZE) // size of a kernel stack guard

// Memory-mapped IO
#define MMIOLIM	(KSTACKTOP - PTSIZE)
#define MMIOBASE	(MMIOLIM - PTSIZE)

#define ULIM		(MMIOBASE)

// below are user read-only mappings till UTOP. They are global pages mapped in at env allocation time.

// user read-only virtual page table
#define UVPT		(ULIM - PTSIZE)
// read-only copies of the Page structures
#define UPAGES	(UVPT - PTSIZE)
// read-only copies of the global env structures
#define UENVS		(UPAGES - PTSIZE)

// top of the user VM, user can manipulate virtual address from UTOP - 1 and down.

// top of the user-accessible VM
#define UTOP 		UENVS
// top of one-page user exception stack
#define UXSTACKTOP	UTOP
// leave empty pages to guard against exception stack overflow

// top pf normal user stack
#define UXSTACKTOP	(UTOP - 2 * PGSIZE)

// user programs begin
#define UTEXT		(2 * PTSIZE)

// temporary page mapping
#define UTEMP		((void*) PTSIZE)
// temporary page mapping for the user-default handler
#define PFTEMP	(UTEMP + PTSIZE - PGSIZE)
// location of the user-level STABS data structure
#define USTABDATA	(PTSIZE / 2) // debug data format storing information about programs symbol

#ifndef __ASSEMBLER__

typedef uint32_t pte_t;
typedef uint32_t pde_t;

#if JOS_USER

// page directory entry (pde) is in the virtual address range [UVPT, UVPT + PTSIZE]
// page directory itself is treated as a page table as well as a page directory

extern volatile pte_t uvpt[]; // virtual address of "virtual page table"
extern volatile pde_t uvpd[]; // virtual address of current page directory
#endif

// page descriptor structure is mapped at UPAGES, it is read/write to the kernel and read-only to user programs
// each PageInfo stores metadata for one physical page but doesn't represent the physical page itself. one-to-one 
// mapping from PageInfo * to physical address using page2pa() in kern/pmap.h
struct PageInfo {
	// next page on the free list
	struct PageInfo *pp_link;

	// pp_ref is the count of pointers (in page table entries) to this page allocated using page_alloc
	// page allocated during boot time with boot_alloc (pmap.c) doesn't have valid reference count field
	uint16_t pp_ref;
};

#endif /* !__ASSEMBLER__*/
#endif


