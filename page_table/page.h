#pragma once 
#include "common.h"
//PTE(Page Table Entry) and SATP
#define  SATP_SV32 (1u<<31) //the first bit in satp is used for determining to turn on page mechanism or not
#define  PAGE_V    (1<<0) // valid bit in PTE
#define  PAGE_R    (1<<1) // read bit in PTE
#define  PAGE_W    (1<<2) // write bit in PTE
#define  PAGE_X    (1<<3) // executable bit in PTE
#define  PAGE_U    (1<<4) // user mode bit in PTE

paddr_t alloc_pages(uint32_t n);

void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags);