#include "kernel.h"
#include "common.h"
extern char __bss[], __bss_end[], __stack_top[];
extern char __free_ram[], __free_ram_end[];
extern char __kernel_base[];
extern char __text_end[];
extern char __rodata_end[];
extern char __data_end[];
struct process *proc_a;
struct process *proc_b;
struct process procs[PROCS_MAX];
struct process *current_process;
struct process *idle_process;

void delay(void)
{
    for (int i = 0; i < 30000000; i++)
    {
        __asm__ __volatile__("nop");
    }
}
void yield(void)
{
    struct process *next = idle_process;
    for (int i = 0; i < PROCS_MAX; i++)
    {
        struct process *proc = &(procs[(current_process->pid + i) % PROCS_MAX]);
        if (proc->state == PROC_RUNABLE && proc->pid > 0)
        {
            next = proc;
            break;
        }
    }
    if (next == current_process)
    {
        return;
    }
    struct process *prev_process = current_process;
    current_process = next;
    __asm__ __volatile__(
        "sfence.vma\n"
        "csrw satp, %[satp]\n"
        "sfence.vma\n"
        "csrw sscratch, %[sscratch]\n"
        :
        : [satp] "r"(SATP_SV32 | ((uint32_t)next->page_table / PAGE_SIZE)),
          [sscratch] "r"((uint32_t)&next->stack[sizeof(next->stack)])
        );

         // char *p=__bss_end-0x1000;
         // memset(p,1,4096);
      /* uint8_t *last=(uint8_t*)__bss_end -1;
       uint32_t page=(uint32_t)last & ~(PAGE_SIZE - 1);
       uint8_t *p=(uint8_t*)page;
       printf("last page of bss: %x",p);
       *p=0xFF;*/
    switch_context(&(prev_process->sp), &(next->sp));
}

struct process *create_process(uint32_t pc)
{
    struct process *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++)
    {
        if (procs[i].state == PROC_UNUSED)
        {
            proc = &procs[i];
            break;
        }
    }
    if (!proc)
    {
        PANIC("no free process slots");
    }
    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context.
    // 這邊只是使用uint8_t表示1 bytes，但轉成uint32_t時會是4 bytes為一個interval
    uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)];
    *--sp = 0;            // s11
    *--sp = 0;            // s10
    *--sp = 0;            // s9
    *--sp = 0;            // s8
    *--sp = 0;            // s7
    *--sp = 0;            // s6
    *--sp = 0;            // s5
    *--sp = 0;            // s4
    *--sp = 0;            // s3
    *--sp = 0;            // s2
    *--sp = 0;            // s1
    *--sp = 0;            // s0
    *--sp = (uint32_t)pc; // ra

    uint32_t *page_table = (uint32_t *)alloc_pages(1);
    printf("page_table: %x\n",page_table);
    for (paddr_t paddr = (paddr_t)__kernel_base; paddr < (paddr_t)__free_ram_end; paddr += PAGE_SIZE)
        map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);
    proc->page_table = page_table;
    // Initialize fields.
    proc->pid = i + 1;
    proc->state = PROC_RUNABLE;
    proc->sp = (uint32_t)sp;
    return proc;
}
__attribute__((naked)) void switch_context(uint32_t *prev_sp, uint32_t *next_sp)
{
    __asm__ __volatile__(
        // Save callee-saved registers onto the current process's stack.
        // s register need to be preserve all over functions call
        "addi sp, sp, -13 * 4\n" // Allocate stack space for 13 4-byte registers
        "sw ra,  0  * 4(sp)\n"   // Save callee-saved registers only
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // Switch the stack pointer.
        "sw sp, (a0)\n"        // *prev_sp = sp; sw source , offset
        "lw sp, (a1)\n"        // Switch stack pointer (sp) here; lw destination, offset
                               // Restore callee-saved registers from the next process's stack.
        "lw ra,  0  * 4(sp)\n" // Restore callee-saved registers only
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n" // We've popped 13 4-byte registers from the stack
        "ret\n"

    );
}

void proc_a_entry(void)
{
    printf("starting process A\n");
    while (1)
    {

        putchar('A');
      uint8_t *last=(uint8_t*)__bss_end -1;
       uint32_t page=(uint32_t)last & ~(PAGE_SIZE - 1);
       uint8_t *p=(uint8_t*)page;
       printf("last page of bss: %x",p);
       *p=0xFF;
        yield();
        delay();
        // switch_context(&proc_a->sp, &proc_b->sp);
        // delay();
    }
}

void proc_b_entry(void)
{
    printf("starting process B\n");
    while (1)
    {
        putchar('B');
        putchar('A');
        /*uint8_t *last=(uint8_t*)__bss_end -1;
         uint32_t page=(uint32_t)last & ~(PAGE_SIZE - 1);
         uint8_t *p=(uint8_t*)page;
         printf("last page of bss: %x",p);
         *p=0xFF;*/
        //       switch_context(&proc_b->sp, &proc_a->sp);
        //      delay();
        yield();
        delay();
    }
}

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid)
{
    // 使用register的directive乃告知compiler這個變數會直接儲存在register，這樣可以少寫mv的assembly code
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    // 這裡為OpenSBI的extension id，對應OpenSBI的CONSOLE_PUTCHAR的function call
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch)
{
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
}

__attribute__((naked))
__attribute__((aligned(4))) void
kernel_entry(void)
{
    __asm__ __volatile__(
        "csrw sscratch, sp\n" // store sp value into sscratch(supervisor sctratch register)
        "addi sp, sp, -4 * 31\n"
        "sw ra,  4 * 0(sp)\n" // return address
        "sw gp,  4 * 1(sp)\n" // global pointer
        "sw tp,  4 * 2(sp)\n" // thread pointer
        "sw t0,  4 * 3(sp)\n" // temporary registers
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n" // function argument registers
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n" // saved registers(calle-saved)
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        "csrr a0, sscratch\n" // read sscratch(sp) into a0(argument 0)
        "sw a0, 4 * 30(sp)\n" // store a0 register value into 4*30 address of sp

        "mv a0, sp\n" // move sp value into a0, the a0 is represent as trap_frame
        "call handle_trap\n"
        // restore the register value
        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n" // call sret to transfer control from s mode into u mode

    );
}

void handle_trap(struct trap_frame *f)
{
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x ,spec=%x", scause, stval, user_pc);
}

void kernel_main(void)
{
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);
    
    WRITE_CSR(stvec, (uint32_t)kernel_entry);
    printf("__text_end :%x\n",__text_end);
    printf("__rodata_end :%x\n",__rodata_end);
    printf("__data_end: %x\n",__data_end);
    printf("__bss: %x\n",__bss);
    printf("__bss_end: %x\n",__bss_end);
    printf("__stack_top: %x\n",__stack_top);
    printf("free_ram: %x\n",__free_ram);
    printf("free_ram_end: %x\n",__free_ram_end);
    idle_process = create_process((uint32_t)NULL);
    idle_process->pid = 0;
    current_process = idle_process;
    proc_a = create_process((uint32_t)proc_a_entry);
    proc_b = create_process((uint32_t)proc_b_entry);
   
    proc_a_entry();
    proc_b_entry();
    yield();
    // PANIC("unreachable error");
    PANIC("switch to idle process");
}

__attribute__((section(".text.boot")))
__attribute__((naked)) void
boot(void)
{
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // Set the stack pointer
        "j kernel_main\n"       // Jump to the kernel main function
        :
        : [stack_top] "r"(__stack_top) // Pass the stack top address as %[stack_top]
    );
}
