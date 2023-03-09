#ifndef _LATX_COUNTER_SYS_H_
#define _LATX_COUNTER_SYS_H_

void latxs_counter_tb_tr(void *cpu);
void latxs_counter_tb_inv(void *cpu);
void latxs_counter_tb_flush(void *cpu);

void latxs_counter_jc_flush(void *cpu);
void latxs_counter_jc_flush_page(void *cpu);

void latxs_counter_helper_store(void *cpu);
void latxs_counter_helper_store_io(void *cpu);
void latxs_counter_helper_store_stlbfill(void *cpu);
void latxs_counter_helper_load(void *cpu);
void latxs_counter_helper_load_io(void *cpu);
void latxs_counter_helper_load_stlbfill(void *cpu);

void latxs_counter_wake(void *cpu);
void latxs_counter_bg_log(int sec);

#endif
