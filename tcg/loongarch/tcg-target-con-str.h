/* SPDX-License-Identifier: MIT */
/*
 * Define RISC-V target-specific operand constraints.
 * Copyright (c) 2021 Linaro
 */

/*
 * Define constraint letters for register sets:
 * REGS(letter, register_mask)
 */
REGS('r', ALL_GENERAL_REGS)
REGS('L', ALL_GENERAL_REGS & ~SOFTMMU_RESERVE_REGS)

/*
 * Define constraint letters for constants:
 * CONST(letter, TCG_CT_CONST_* bit set)
 */
CONST('I', TCG_CT_CONST_S12)
CONST('N', TCG_CT_CONST_N12)
CONST('M', TCG_CT_CONST_M12)
CONST('Z', TCG_CT_CONST_ZERO)
CONST('U', TCG_CT_CONST_U12)
