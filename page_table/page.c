#include "page.h"

extern char __free_ram[], __free_ram_end[];
paddr_t alloc_pages(uint32_t n)
{
    static paddr_t next_paddr = (paddr_t)__free_ram;
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;
    if (next_paddr > (paddr_t)__free_ram_end)
        PANIC("out of memory");
    memset((void *)paddr, 0, n * PAGE_SIZE);
    return paddr;
}

void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags){
    // Ensure vaddr and paddr are aligned to page boundaries (typically 4KB)
    // Since this function deals with page mapping, sub-page offset is not relevant.
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x", vaddr);
    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x", paddr);

    // In RISC-V Sv32, virtual address has 2-level page table lookup:
    // [VPN1 (bits 31-22)] -> root-level page table (table1)
    // [VPN0 (bits 21-12)] -> second-level page table (table0)
    // [page offset (bits 11-0)] is not used in this function.
    uint32_t vpn1 = (vaddr >> 22) & 0x3ff;

    // If the root-level PTE (VPN1) is invalid, allocate a second-level page table (table0)
    if ((table1[vpn1] & PAGE_V) == 0) {
        uint32_t pt_addr = alloc_pages(1); // Allocate 1 page = 4KB for 1024 entries
        // Encode the physical page number of table0 into the root-level PTE
        // Shift left by 10 to match the RISC-V PTE format (PPN[19:0] << 10)
        table1[vpn1] = ((pt_addr / PAGE_SIZE) << 10) | PAGE_V;
    }

    // Extract VPN0 (bits 21-12) — index into second-level page table
    uint32_t vpn0 = (vaddr >> 12) & 0x3ff;

    // Reconstruct physical address of second-level page table (table0)
    // table1[vpn1] holds (PPN << 10), so right-shift 10 and multiply by PAGE_SIZE
    uint32_t *table0 = (uint32_t *)((table1[vpn1] >> 10) * PAGE_SIZE);

    // Map VPN0 entry to point to the actual physical page (leaf PTE)
    // This PTE does **not** point to another page table — it points to a physical page.
    // That's because we're using RISC-V Sv32: the second-level PTE is always a **leaf**
    table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V;
}