#include <kernel/io.h>
#include "gdt.h"

void kernel_main()
{
    gdt_install();

    cls();
    puts("Boot !");

    while(1);
}
