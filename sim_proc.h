#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

typedef struct RMT_table
{
    int is_valid;
    int tag; 
} RMT_table;

typedef struct instructions
{
    int src1, src2, dst, tag, src1_ini, src2_ini, old, src1_ROB, src2_ROB, dst_ini;
    bool sr1_ready;
    bool sr2_ready;
    int dst_rdy;
    int is_valid;
    int op_type;
    int delay;
    int ready_bit;
    int age;
    int FE_start, FE_cycle, DE_start, DE_cycle, RN_start, RN_cycle, RR_start, RR_cycle, 
        DI_start, DI_cycle, IS_start, IS_cycle, EX_start, EX_cycle, WB_start, WB_cycle, RT_start, RT_cycle;
}instructions;

#endif
