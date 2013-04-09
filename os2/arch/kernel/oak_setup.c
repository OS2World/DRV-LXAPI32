/* $Id: oak_setup.c,v 1.6 2005/06/05 22:51:58 smilcke Exp $ */

/*
 * oak_setup.c
 * Autor:               Stefan Milcke
 * Erstellt am:         16.08.2004
 * Letzte Aenderung am: 05.06.2005
 *
*/

#include <lxcommon.h>
#include <linux/init.h>
#include <linux/setup.h>
#include <linux/string.h>
#include <asm/e820.h>

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/apm_bios.h>
#include <linux/initrd.h>
#include <linux/bootmem.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/root_dev.h>
#include <linux/highmem.h>
#include <linux/module.h>
#include <linux/efi.h>
#include <linux/init.h>
#include <linux/edd.h>
#include <video/edid.h>
#include <asm/e820.h>
#include <asm/mpspec.h>
#include <asm/setup.h>
#include <asm/arch_hooks.h>
#include <asm/sections.h>
#include <asm/io_apic.h>
#include <asm/ist.h>
#include <asm/std_resources.h>

/* This value is set up by the early boot code to point to the value
   immediately after the boot time page tables.  It contains a *physical*
   address, and must not be in the .bss segment! */
unsigned long init_pg_tables_end __initdata = ~0UL;

struct cpuinfo_x86 new_cpu_data __initdata = { 0, 0, 0, 0, -1, 1, 0, 0, -1 };
struct cpuinfo_x86 boot_cpu_data = { 0, 0, 0, 0, -1, 1, 0, 0, -1 };
struct e820map e820;
struct screen_info screen_info={0};

static char command_line[COMMAND_LINE_SIZE];
char saved_command_line[COMMAND_LINE_SIZE];
int disable_pse __initdata = 0;

//-------------------------------- setup_memory --------------------------------
static unsigned long __init setup_memory(void)
{
// init_bootmem(start_pfn,max_low_pfn);
 return 0;
}

//--------------------------------- setup_arch ---------------------------------
void __init setup_arch(char **cmdline_p)
{
 init_mm.start_code=(unsigned long) _text;
 init_mm.end_code=(unsigned long) _etext;
 init_mm.end_data=(unsigned long) _edata;
// init_mm.brk=init_pg_tables_end+PAGE_OFFSET;
 memcpy(&boot_cpu_data, &new_cpu_data, sizeof(new_cpu_data));
 // TODO(SM): fill in boot_cpu_data correctly
 strcpy(command_line,saved_command_line);
 *cmdline_p=command_line;

// max_low_pfn=setup_memory();
 setup_memory();

 // CONFIG_EARLY_PRINTK
/*
#ifdef CONFIG_VT
#if defined(CONFIG_VGA_CONSOLE)
   if (!efi_enabled || (efi_mem_type(0xa0000) != EFI_CONVENTIONAL_MEMORY))
      conswitchp = &vga_con;
#elif defined(CONFIG_DUMMY_CONSOLE)
   conswitchp = &dummy_con;
#endif
#endif
*/
}

//----------------------------- apply_alternatives -----------------------------
void apply_alternatives(void *start,void *end)
{
}

//-------------------------- alternative_instructions --------------------------
void __init alternative_instructions(void)
{
}
