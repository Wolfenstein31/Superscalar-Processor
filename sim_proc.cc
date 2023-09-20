#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_proc.h"
#include <iostream>
#include <inttypes.h>
#include <bits/stdc++.h>
#include <vector>
using namespace std;


int WIDTH, ROB_SIZE, IQ_SIZE;
int op_type, dest, src1, src2;
unsigned long int pc;
bool available;
int rob_tail, rob_head;
FILE *FP;  
int current_cycle = 0;
int total_num_cycles = 0;
int counter_instr = 0;
int age = 0;


bool stage_empty(vector<instructions> &stage)
{
    for (int i = 0; i<WIDTH; i++)
    {
        if (stage[i].is_valid == 1)
        return false;
    }
    return true;
}

bool ROB_empty(vector<instructions> &stage)
{
    for (int i = 0; i<ROB_SIZE; i++)
    {
        if (stage[i].is_valid == 1)
        return false;
    }
    return true;
}

bool iq_empty(vector<instructions> &stage)
{
    for (int i = 0; i<IQ_SIZE; i++)
    {
        if (stage[i].is_valid == 1)
        return false;
    }
    return true;
}

bool rmt_empty(vector<RMT_table> &stage)
{
    for (int i = 0; i<67; i++)
    {
        if (stage[i].is_valid == 1)
        return false;
    }
    return true;
}

vector<instructions> DE;
vector<instructions> RN;
vector<instructions> RR;
vector<instructions> DI;
vector<instructions> IQ;
vector<instructions> EX;
vector<instructions> WB;
vector<instructions> ROB;
vector<RMT_table> RMT;

void fetch()
{
    if (stage_empty(DE) == true)
    {
        int i =0;
        while(i<WIDTH)
        {
            if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
            {
                DE[i].is_valid=1;
                DE[i].src1 = src1;
                DE[i].src1_ini = src1;
                
                DE[i].src2 = src2;
                DE[i].src2_ini = src2;

                DE[i].dst = dest;
                DE[i].dst_ini = dest;
                
                DE[i].dst_rdy = 0;
                DE[i].age = age;

                if(DE[i].src1 != -1)
                {
                    DE[i].sr1_ready = false;
                }
                else
                {
                    DE[i].sr1_ready = true;
                }

                
                if(DE[i].src2 != -1)
                {
                    DE[i].sr2_ready = false;
                }
                else
                {
                    DE[i].sr2_ready = true;
                }

                DE[i].op_type = op_type;
                switch(op_type)
                {
                    case 0: DE[i].delay = 1;
                    break;
                    case 1: DE[i].delay =2;
                    break;
                    case 2: DE[i].delay = 5;
                    break;
                    default: DE[i].delay = 0;
                }

                DE[i].FE_start = current_cycle;
                DE[i].FE_cycle = 1;
                DE[i].DE_start = current_cycle+1;
                age++;
            }
            i++;
        }
    }
    current_cycle++;
}

void decode()
{
    if(stage_empty(RN) == true)
    {
        int i = 0;
        while( i< WIDTH)
        {
            if(DE[i].is_valid == 1)
            {
                DE[i].is_valid = 0;
                RN[i] = DE[i];
                RN[i].is_valid = 1;
                RN[i].RN_start = current_cycle+1;
                RN[i].DE_cycle = RN[i].RN_start - RN[i].DE_start;
            }
            i++;
        }
    }
}

void src1_check(int &source1)
{
    if(source1!= -1)
    {
        if(RMT[source1].is_valid == 1)
        {
            source1 = RMT[source1].tag;
        }
    }
}

void src2_check(int &source2)
{
    if(source2!= -1)
    {
        if(RMT[source2].is_valid == 1)
        {
            source2 = RMT[source2].tag;
        }
    }
}

void dst_check(int &destination)
{
    if(destination != -1)
    {
        RMT[destination].is_valid = 1;
        RMT[destination].tag = rob_tail + 1000;
        destination = rob_tail+1000;
    }

    else
    {
        destination = rob_tail+1000;
    }

    if(rob_tail == ROB_SIZE - 1)
    {
        rob_tail = 0;
    }
    else
    {
        rob_tail++;
    }
}
void rename()
{
    int free_width = 0;
    int flag =0;
    int j = 0;
    while(j < ROB_SIZE)
    {
        if(ROB[j].is_valid == 0)
        {
            free_width++;
            
        }
        j++;
    }

    
    if(free_width >= WIDTH)
    {
          flag =1;
    }
    else flag =0;
    
    if((stage_empty(RR) == true) && flag ==1)
    {
        int i = 0;
        while( i < WIDTH)
        {
            if(RN[i].is_valid == 1)
            {
                
                ROB[rob_tail] = RN[i];
                RN[i].is_valid = 0;

                src1_check(RN[i].src1);

                src2_check(RN[i].src2);

                dst_check(RN[i].dst);

                RR[i] = RN[i];
                RR[i].is_valid = 1;
                RR[i].RR_start = current_cycle+1;
                RR[i].RN_cycle = RR[i].RR_start - RR[i].RN_start;
            }
        i++;
        }
    }
}
void check_src1_ready(int &a, bool &b)
{
     if(a!= -1)
        {
            if(a >= 1000) //checking the ROB
            {
                if(ROB[a-1000].dst_rdy == 1)
                {
                    b = true;
                }
            }

                else if(a < 67) //If not in ROB checking the ARF
                {
                    b = true;
                }
        
            else if(a == -1) //implies there is no source reg
            {
                b = true;
            }
        }
}
void check_src2_ready(int &a, bool &b)
{
    if(a!= -1)
        {
            if(a >= 1000) //checking the ROB
            {
                if(ROB[a-1000].dst_rdy == 1)
                {
                    b = true;
                }
            }

            else if(a < 67) //If not in ROB checking the ARF
            {
                b = true;
            }
        
            else if(a == -1) //implies there is no source reg
            {
                b = true;
            }
        }
}
void reg_read()
{
    if(stage_empty(DI))
    {

        for(int i = 0; i < WIDTH; i++)
        {
            if(RR[i].is_valid == 1)
            {
                RR[i].is_valid = 0;
                
                check_src1_ready(RR[i].src1,RR[i].sr1_ready);
                check_src2_ready(RR[i].src2,RR[i].sr2_ready);
                DI[i] = RR[i];
                DI[i].is_valid = 1;
                DI[i].DI_start = current_cycle+1;
                DI[i].RR_cycle = DI[i].DI_start - DI[i].RR_start;
            }
        }
            
    }

    }


int iq_free_check(vector<instructions> iq)
{
    int iq_free_entries = 0;
    int k=0;
    while (k< IQ_SIZE)
    {
        if(iq[k].is_valid == 0)
         {
             iq_free_entries++;
         }
        k++;
    }
    return iq_free_entries;
}

int di_entry_check(vector<instructions> di)
{
    int di_entries = 0;
    int l=0;
    while(l<WIDTH)
    {
        if(di[l].is_valid == 1)
         {
             di_entries++;
         }
         l++;
    }
    return di_entries;
}

void dispatch()
{
    int flag =0;
    int iq_free = iq_free_check(IQ);
    int di_free = di_entry_check(DI);

    if (iq_free >= di_free)
    {
        flag =1;
    }
    else flag =0;
    
    if(flag ==1)
    {
        int i = 0;
        while(i<WIDTH)
        {
             if(DI[i].is_valid == 1)
             {
                int j = 0;
                while(j < IQ_SIZE)
                {
                     if(IQ[j].is_valid == 0)
                    {
                        DI[i].is_valid = 0;
                        IQ[j] = DI[i];
                        IQ[j].is_valid = 1;

                        IQ[j].IS_start = current_cycle+1;
                        IQ[j].DI_cycle = IQ[j].IS_start - IQ[j].DI_start;
                        break;
                    }
                    j++;
                }
             }
             i++;
        }
    }
}

void issue_sort()
{
    int i = 0;
    while(i < IQ_SIZE)
    {   
        int j = i+1;
        while( j < IQ_SIZE)
        {
            if(IQ[i].age > IQ[j].age)
            {
                swap(IQ[i],IQ[j]);
            }
            j++;
        }
        i++;
    }
}

int update_issue(int i,int j,int instr_count)
{
    EX[j] = IQ[i];
    IQ[i].is_valid = 0;
    EX[j].is_valid = 1;
    
    EX[j].EX_start = current_cycle+1;
    EX[j].IS_cycle = EX[j].EX_start - EX[j].IS_start;

    instr_count++;
    return instr_count;

}

void issue()
{
    
    issue_sort();
    int instr_count = 0;
    int i = 0;
    while(i< IQ_SIZE)
    {
        if(IQ[i].is_valid == 1)
        {
            if(IQ[i].sr1_ready == true && IQ[i].sr2_ready == true)
            {
                int j = 0;
                while(j< WIDTH*5)
                {
                    if(EX[j].is_valid == 0)
                    {
                        instr_count = update_issue(i,j,instr_count);
                        if(instr_count == WIDTH)
                        {
                            return;
                        }
                        
                        break;
                    }
                    j++;
                }
            }
        }
        i++;
    }
}

void iq_wakeup(int &destination)
{
    for(int k = 0; k < IQ_SIZE; k++)
                {
                    if(IQ[k].is_valid == 1)
                    {
                        if(destination == IQ[k].src1)
                        {
                            if(IQ[k].dst != -1)
                            {
                                IQ[k].sr1_ready = true;
                            }
                        }
                    }

                    if(IQ[k].is_valid == 1)
                    {
                        if(destination == IQ[k].src2)
                        {
                            if(IQ[k].dst != -1)
                            {
                                IQ[k].sr2_ready = true;
                            }
                        }
                    }
                }

}
void di_wakeup(int &destination)
{
    int k=0;
    while(k < WIDTH)
    {
        if(DI[k].is_valid == 1)
        {
            if(destination == DI[k].src1)
            {
                if(destination != -1)
                {
                    DI[k].sr1_ready = true;
                }
            }
        }

        if(DI[k].is_valid == 1)
        {
            if(destination == DI[k].src2)
            {
                if(destination != -1)
                {
                    DI[k].sr2_ready = true;
                }
            }
        }
        k++;
    }
}

void rr_wakeup(int &destination)
{
    int k=0;
    while(k < WIDTH)
    {
        if(RR[k].is_valid == 1)
        {
            if(destination == RR[k].src1)
            {
                if(RR[k].dst != -1)
                {
                    RR[k].sr1_ready = true;
                }
            }
        }

        if(RR[k].is_valid == 1)
        {
            if(destination == RR[k].src2)
            {
                if(RR[k].dst != -1)
                {
                    RR[k].sr2_ready = true;
                }
            }
        }
        k++;
    }
}

void writeback_updation(instructions &exec)
{
    int j=0;
    while(j < WIDTH * 5)
    {
        if(WB[j].is_valid == 0)
            {
                WB[j] = exec;
                WB[j].is_valid = 1;
                WB[j].dst_rdy = 1;
                exec.is_valid = 0;
                        
                WB[j].WB_start = current_cycle+1;
                WB[j].EX_cycle = WB[j].WB_start - WB[j].EX_start;
                break;
            }
        j++;
    }
}
void execute()
{
    for(int i = 0; i < WIDTH * 5; i++)
    {

        //putting the data in the WB buffer
        if(EX[i].is_valid == 1)
        {
            EX[i].delay--;
            if(EX[i].delay == 0)
            {
                iq_wakeup(EX[i].dst);

                //DI wakeup
                di_wakeup(EX[i].dst);

                //RR wakeup
                rr_wakeup(EX[i].dst);

                //Writeback updation
                writeback_updation(EX[i]);
            }
        }
    }
}


void writeback()
{
    for(int i = 0; i < WIDTH * 5; i++)
    {
        if(WB[i].is_valid == 1)
        {
            WB[i].is_valid = 0;
            ROB[WB[i].dst - 1000] = WB[i];
            ROB[WB[i].dst - 1000].dst_rdy = 1;
            ROB[WB[i].dst - 1000].is_valid = 1;

            ROB[WB[i].dst - 1000].RT_start = current_cycle+1;
            ROB[WB[i].dst - 1000].WB_cycle = ROB[WB[i].dst - 1000].RT_start - ROB[WB[i].dst - 1000].WB_start;

        }
    }
}

void rmt_rob_updation()
{
    if(ROB[rob_head].dst_ini != -1 && RMT[ROB[rob_head].dst_ini].is_valid == 1 && RMT[ROB[rob_head].dst_ini].tag == rob_head + 1000)
    {
        RMT[ROB[rob_head].dst_ini].is_valid = 0;

    }
}
void retire()
{
    int i=0;
    while(i < WIDTH)
    {
         if((ROB[rob_head].dst_rdy == 1) && ROB[rob_head].is_valid == 1)
         {
            rmt_rob_updation();

            ROB[rob_head].RT_cycle = (current_cycle + 1)-ROB[rob_head].RT_start;
            ROB[rob_head].is_valid = 0;

            total_num_cycles = ROB[rob_head].RT_start +  ROB[rob_head].RT_cycle;

            printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",
                    counter_instr,
                    ROB[rob_head].op_type,
                    ROB[rob_head].src1_ini, ROB[rob_head].src2_ini,
                    ROB[rob_head].dst_ini,
                    ROB[rob_head].FE_start, ROB[rob_head].FE_cycle,
                    ROB[rob_head].DE_start, ROB[rob_head].DE_cycle,
                    ROB[rob_head].RN_start, ROB[rob_head].RN_cycle,
                    ROB[rob_head].RR_start, ROB[rob_head].RR_cycle,
                    ROB[rob_head].DI_start, ROB[rob_head].DI_cycle,
                    ROB[rob_head].IS_start, ROB[rob_head].IS_cycle,
                    ROB[rob_head].EX_start, ROB[rob_head].EX_cycle,
                    ROB[rob_head].WB_start, ROB[rob_head].WB_cycle,
                    ROB[rob_head].RT_start, ROB[rob_head].RT_cycle);
            
            counter_instr++;        
            if(rob_head == ROB_SIZE-1)
            {
                rob_head = 0;
            }
            else
            {
                rob_head++;
            }
        }

        else
        {
            return;
        }
        i++;
    }
}

bool advance_cycle()
{
    if(stage_empty(DE) && 
       stage_empty(RN) &&
       stage_empty(RR) &&
       stage_empty(DI) &&
       stage_empty(IQ) &&
       stage_empty(EX) &&
       stage_empty(WB) &&
       ROB_empty(ROB))
       {
        return false;
       }
       return true;
}

int main (int argc, char* argv[])
{
   // FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
   // int op_type, dest, src1, src2;  // Variables are read from trace file
  //  unsigned long int pc; // Variable holds the pc read from input file

    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    // printf("rob_size:%lu "
    //         "iq_size:%lu "
    //         "width:%lu "
    //         "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    WIDTH = params.width;
    ROB_SIZE = params.rob_size;
    IQ_SIZE = params.iq_size;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
        //printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
/////

    DE.resize(WIDTH);
    RN.resize(WIDTH);
    RR.resize(WIDTH);
    DI.resize(WIDTH);
    IQ.resize(IQ_SIZE);
    ROB.resize(ROB_SIZE);
    RMT.resize(67);
    EX.resize(WIDTH*5);
    WB.resize(WIDTH*5);
    rob_head = 0;
    rob_tail = 0;

    int i = 0;
    while(i<WIDTH)
    {
        DE[i].is_valid = 0;
        RN[i].is_valid = 0;
        RR[i].is_valid = 0;
        DI[i].is_valid = 0;
        i++;
    }

    int j = 0;
    while(j < IQ_SIZE)
    {
        IQ[j].is_valid = 0;
        j++;
    }

    j =0;
    while(j<WIDTH * 5)
    {
        EX[j].is_valid = 0;
        WB[j].is_valid = 0;
        j++;
    }


    i =0;
    while(i < ROB_SIZE)
    {
        ROB[i].is_valid = 0;
        ROB[i].dst_rdy = 0;
        i++;
    }

    i=0;
    while(i < 67)
    {
        RMT[i].is_valid = 0;
        RMT[i].tag = i;
        i++;
    }

    do
    {
        retire();
        writeback();
        execute();
        issue();
        dispatch();
        reg_read();
        rename();
        decode();
        fetch();
    } while(advance_cycle());

    cout<< "# === Simulator Command ========="<<endl;
    printf("# ./sim %lu %lu %d %s\n", ROB_SIZE, IQ_SIZE, WIDTH, argv[4]);
    cout<<"# === Processor Configuration ==="<<endl;
    cout<<"# ROB_SIZE = "<<ROB_SIZE<<endl;
    cout<<"# IQ_SIZE = "<<IQ_SIZE<<endl;
    cout<<"# WIDTH = "<<WIDTH<<endl;
    cout<< "# === Simulation Results ========"<<endl;
    cout<<"# Dynamic Instruction Count    = "<<counter_instr<<endl;
    cout<<"# Cycles                       = "<<total_num_cycles<<endl;
    cout<<"# Instructions Per Cycle (IPC) = "<<fixed<<setprecision(2)<<((float)counter_instr/(float)total_num_cycles)<<endl;
    return 0;
}
