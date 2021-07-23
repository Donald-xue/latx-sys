#include "common.h"
#include "reg-alloc.h"
#include "ir2.h"
#include "latx-options.h"
#include "lsenv.h"
#include "translate.h"

void latxs_load_imm64(IR2_OPND *opnd2, int64_t value)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    if (value >> 11 == -1 || value >> 11 == 0) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, opnd2, zero, value);
    } else if (value >> 12 == 0) {
        latxs_append_ir2_opnd2i(LISA_ORI, opnd2, zero, value);
    } else if (value >> 31 == -1 || value >> 31 == 0) {
        latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        if (value & 0xfff) {
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
    } else if (value >> 32 == 0) {
        latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        if (value & 0xfff) {
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
        latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, 0);
    } else if (value >> 51 == -1 || value >> 51 == 0) {
        if ((value & 0xffffffff) == 0) {
            latxs_append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
        } else if ((value & 0xfff) == 0) {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        } else {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
        latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
    } else {
        if ((value & 0xfffffffffffff) == 0) {
            latxs_append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
        } else if ((value & 0xffffffff) == 0) {
            latxs_append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
            latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
        } else if ((value & 0xfff) == 0) {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        } else {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
            latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
        }
        latxs_append_ir2_opnd2i(LISA_LU52I_D,
                opnd2, opnd2, (value >> 52) & 0xfff);
    }
}

void latxs_load_imm32_to_ir2(IR2_OPND *opnd2, uint32_t value, EXMode em)
{
    lsassertm(latxs_ir2_opnd_is_gpr(opnd2),
            "load imm 32 to ir2: IR2 OPND is not GPR\n");
    latxs_load_imm64(opnd2, value);
}

void latxs_load_imm64_to_ir2(IR2_OPND *opnd2, uint64_t value)
{
    lsassertm(latxs_ir2_opnd_is_gpr(opnd2),
            "load imm 64 to ir2: IR2 OPND is not GPR\n");

    int32 high_32_bits = value >> 32;
    int32 low_32_bits = value;

    if (high_32_bits == 0) {
        /* 1. 0000 0000 xxxx xxxx */
        latxs_load_imm32_to_ir2(opnd2, low_32_bits, EXMode_Z);
        return;
    } else if (high_32_bits == -1) {
        /* 2. ffff ffff xxxx xxxx */
        if (low_32_bits < 0) {
            latxs_load_imm32_to_ir2(opnd2, low_32_bits, EXMode_S);
            return;
        } else {
            latxs_load_imm32_to_ir2(opnd2, ~low_32_bits, EXMode_Z);
            latxs_append_ir2_opnd3(LISA_NOR, opnd2, opnd2, opnd2);
            return;
        }
    } else {
        /* 3. xxxx xxxx xxxx xxxx */
        latxs_load_imm64(opnd2, value);
        return;
    }
}

void latxs_load_addrx_to_ir2(IR2_OPND *opnd, ADDRX addrx)
{
    latxs_load_imm32_to_ir2(opnd, (uint32_t)addrx, EXMode_Z);
}

void latxs_load_addr_to_ir2(IR2_OPND *opnd, ADDR addr)
{
    latxs_load_imm64_to_ir2(opnd, (uint64_t)addr);
}

static IR2_OPND latxs_convert_mem_ir2_opnd_plus_offset(
        IR2_OPND *mem, int addend)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    IR2_OPND mem_opnd = *mem;

    int mem_offset = latxs_ir2_opnd_imm(mem);

    /* addend = 2 : 0x7fe */
    /* addend = 4 : 0x7fc */
    int mem_offset_high = ((int)0x800) - addend;

    /* situation 1: >> directly adjust mem_offset in IR2_OPND */
    /* situation 2: >> directly adjust base register in IR2_OPND */
    /* situation 3: >> use a new temp register as new base */
    if (likely(mem_offset < mem_offset_high)) {
        /* situation 1 */
        latxs_ir2_opnd_mem_adjust_offset(&mem_opnd, addend);
    } else {
        IR2_OPND mem_old_base = latxs_ir2_opnd_mem_get_base(mem);
        if (latxs_ir2_opnd_is_itemp(&mem_old_base)) {
            /* situation 2 */
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_old_base,
                    &mem_old_base, addend);
        } else {
            /* situation 3 */
            IR2_OPND mem_new_base = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_new_base,
                    &mem_old_base, addend);
            latxs_ir2_opnd_mem_set_base(&mem_opnd, &mem_new_base);
        }

    }

    return mem_opnd;
}

IR2_OPND latxs_convert_mem_ir2_opnd_plus_2(IR2_OPND *mem)
{
    return latxs_convert_mem_ir2_opnd_plus_offset(mem, 2);
}
IR2_OPND latxs_convert_mem_ir2_opnd_plus_4(IR2_OPND *mem)
{
    return latxs_convert_mem_ir2_opnd_plus_offset(mem, 4);
}

/*
 * @mem                        return mem_no_offset
 * --------------------       ------------------------
 *    GPR   |  offset     =>       GPR     |  offset
 * ---------+----------       -------------+----------
 *  mapping |  no                new temp  |  no
 *  mapping |  yes               new temp  |  no
 *   temp   |  no                old temp  |  no
 *   temp   |  yes               new temp  |  no
 * ---------+----------       -------------+----------
 */
IR2_OPND latxs_convert_mem_ir2_opnd_no_offset(IR2_OPND *mem, int *newtmp)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    IR2_OPND mem_opnd;

    int mem_offset = latxs_ir2_opnd_mem_get_offset(mem);

    IR2_OPND mem_old_base = latxs_ir2_opnd_mem_get_base(mem);
    IR2_OPND mem_new_base;

    if (mem_offset) {
        mem_new_base = latxs_ra_alloc_itemp();
        *newtmp = 1;
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_new_base,
                &mem_old_base, mem_offset);
    } else {
        if (latxs_ir2_opnd_is_itemp(&mem_old_base)) {
            *newtmp = 0;
            mem_new_base = mem_old_base;
        } else {
            mem_new_base = latxs_ra_alloc_itemp();
            *newtmp = 1;
            latxs_append_ir2_opnd3(LISA_OR, &mem_new_base,
                    &mem_old_base, &latxs_zero_ir2_opnd);
        }
    }

    latxs_ir2_opnd_build_mem(&mem_opnd,
            latxs_ir2_opnd_reg(&mem_new_base), /* base   */
            0);                                /* offset */

    return mem_opnd;
}
