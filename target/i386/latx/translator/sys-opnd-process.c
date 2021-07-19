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
