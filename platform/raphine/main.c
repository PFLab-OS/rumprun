/*
 *
 * Copyright (c) 2017 Raphine Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Author: Liva
 * 
 */

#include <raphine/kernel.h>

#include <bmk-core/core.h>
#include <bmk-core/mainthread.h>
#include <bmk-core/sched.h>
#include <bmk-core/printf.h>
#include <bmk-core/pgalloc.h>

RaphineRing0AppInfo *app_info;

extern int __raphine_module_info_start;

void raphine_putc(int c) {
  app_info->putc(c);
}

int raphine_main(int argc, const char **argv) {
  extern char _end[];
  unsigned long osend;

  app_info = (RaphineRing0AppInfo *)&__raphine_module_info_start;
  if (app_info->version != ((uint64_t)1)<<32) {
    // invalid version
    return -1;
  }

  bmk_printf_init(raphine_putc, NULL);

  bmk_sched_init();

  bmk_core_init(BMK_THREAD_STACK_PAGE_ORDER);

  osend = bmk_round_page((unsigned long)_end);
  bmk_assert(osend < 0x40000000);
  bmk_pgalloc_loadmem(osend, 0x40000000);

  bmk_memsize = 0x40000000 - osend;

  char cmdline[2] = {0};
  bmk_sched_startmain(bmk_mainthread, cmdline);
  
  while(1) {
    asm volatile("hlt");
  }

  return 0;
}
