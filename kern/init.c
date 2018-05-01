#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/monitor.h>
#include <kern/console.h>
#include <kern/pmap.h>
#include <kern/kclock.h>

void i386_init(void)
{
    extern char edata[], end[];
    // set edata to be 0 with a size of end - edata
    // clear BSS section and set all static/global variables start out zero
    memset(edata, 0, end - edata);

    // init console
    cons_init();

    cprintf("This kernel verison decimal is %o octal!\n", 6828);

    // init memory management
    mem_init();

    // Get into the kernel monitor
    while (1)
        monitor(NULL);
}

// first call to panic, used as flag that kernel has called panic
const char *panicstr;

// panic is called on unresolvable fatal errors, in format "panic: mesg"
void _panic(const char *file, int line, const char *fmt, ...)
{
    va_list ap;

    if (panicstr)
        goto dead;
    panicstr = fmt;

    // cli: clear interrupt flag; cld: cld: clear direction flag
    asm volatile("cli; cld");

    // va_start, undefined input type ..., response if determined by va_arg
    va_start(ap, fmt); 
    cprintf("kernel panic at %s:%d: ", file, line);
    vcprintf(fmt, ap);
    cprintf("\n");
    va_end(ap);

dead:
    // get into the kernel monitor
    while (1)
        monitor(NULL);
}

void _warn(const char *file, int line, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    cprintf("kernel warning at %s:%d: ", file, line);
    vcprintf(fmt, ap);
    cprintf("\n");
    va_end(ap);
}
