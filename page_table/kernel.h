#pragma once
#include "common.h"
#include "page.h"

#define PROCS_MAX 8 //Maximum number of processes

#define PROC_UNUSED 0 //Unused process control structure
#define PROC_RUNABLE 1 //Runuable process





struct process{
    int pid; //Process ID
    int state; //Proess state: PROC_UNUSED or PROC_RUNNABLE
    vaddr_t sp; //stack pointer
    uint32_t *page_table;
    uint8_t stack[8192]; //kernel stack
};




struct sbiret {
    long error;
    long value;
};

struct trap_frame {
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));//don't add padding for this structure


//macro automatically return with the last expression
#define READ_CSR(reg)                                                          \
    ({                                                                         \
        unsigned long __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })

#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uint32_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)


struct process *create_process(uint32_t pc);
void delay(void);
void proc_a_entry(void);
void proc_b_entry(void);

void putchar(char ch);
void handle_trap(struct trap_frame *f);
void kernel_main(void);
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid);
__attribute__((naked)) __attribute__((aligned(4))) void kernel_entry(void);
__attribute__((naked)) void switch_context(uint32_t *prev_sp, uint32_t *next_sp);
__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void);
void yield();
