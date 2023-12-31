/*
 *  Translated block handling
 *
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TRANSLATE_ALL_H
#define TRANSLATE_ALL_H

#include "exec/exec-all.h"


/* translate-all.c */
struct page_collection *page_collection_lock(tb_page_addr_t start,
                                             tb_page_addr_t end);
void page_collection_unlock(struct page_collection *set);
void tb_invalidate_phys_page_fast(struct page_collection *pages,
                                  tb_page_addr_t start, int len,
                                  uintptr_t retaddr);
void tb_invalidate_phys_page_range(tb_page_addr_t start, tb_page_addr_t end);
void tb_check_watchpoint(CPUState *cpu, uintptr_t retaddr);

#ifdef CONFIG_USER_ONLY
int page_unprotect(target_ulong address, uintptr_t pc);
int is_epi_pc(uint32_t *inst);
int is_shd_wrt(unsigned long addr);
int is_handling_shd_wrt(void);
void shd_wrt_pro(unsigned long addr, uint64_t next_pc); /* host addr */
void shd_wrt_epi(uint32_t *pc); /* host addr */
#endif
#ifdef CONFIG_LATX
void tb_exit_to_qemu(CPUArchState *env, uintptr_t pc);
#endif
#endif /* TRANSLATE_ALL_H */
