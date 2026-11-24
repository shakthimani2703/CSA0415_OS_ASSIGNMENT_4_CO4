/*
 * Task 2.3 / 2.4 - Cost calculations for a 4 GB file, inode maximum size,
 * free-space structures for a 4 TB disk.
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -o calc_cost calc_cost.c
 */
#include <stdio.h>

int main(void)
{
    const unsigned long long KB = 1024ULL, MB = KB * KB, GB = MB * KB, TB = GB * KB;
    const unsigned long long block = 4 * KB, file = 4 * GB;
    const unsigned long long ptrs = block / 4;               /* 4-byte addresses */
    unsigned long long nblocks = file / block;
    unsigned long long last = nblocks - 1;                   /* logical index of last block */

    printf("=== Task 2.3: reaching the last block of a 4 GB file (4 KB blocks) ===\n");
    printf("Blocks in file                 : %llu\n", nblocks);
    printf("Contiguous                     : 1 disk read (address = start + %llu)\n", last);
    printf("Linked (pointer in each block) : %llu disk reads\n", nblocks);
    unsigned long long fat_blocks = (nblocks * 4 + block - 1) / block;
    printf("FAT on disk (not cached)       : %llu FAT-block reads + 1 data read = %llu\n",
           fat_blocks, fat_blocks + 1);
    printf("FAT cached in memory           : %llu memory look-ups + 1 data read\n", last);

    unsigned long long d = 12, s = ptrs, dd = ptrs * ptrs, td = ptrs * ptrs * ptrs;
    const char *level;
    int reads;
    if (last < d)                { level = "direct";          reads = 2; }
    else if (last < d + s)       { level = "single indirect"; reads = 3; }
    else if (last < d + s + dd)  { level = "double indirect"; reads = 4; }
    else                         { level = "triple indirect"; reads = 5; }
    printf("Indexed (inode)                : block %llu lies in the %s range\n", last, level);
    printf("                                 %d disk reads incl. inode (%d if inode cached)\n",
           reads, reads - 1);

    printf("\n=== Inode maximum file size (12 direct, 1 single, 1 double, 1 triple) ===\n");
    printf("Pointers per block             : %llu\n", ptrs);
    printf("Direct          : %14llu blocks = %llu KB\n", d, d * block / KB);
    printf("Single indirect : %14llu blocks = %llu MB\n", s, s * block / MB);
    printf("Double indirect : %14llu blocks = %llu GB\n", dd, dd * block / GB);
    printf("Triple indirect : %14llu blocks = %llu TB\n", td, td * block / TB);
    unsigned long long total = d + s + dd + td;
    printf("Total           : %14llu blocks = %llu bytes (~%.4f TB)\n",
           total, total * block, (double)(total * block) / (double)TB);

    printf("\n=== Task 2.4: free-space management for a 4 TB disk ===\n");
    unsigned long long disk_blocks = 4 * TB / block;
    printf("Blocks on disk                 : %llu\n", disk_blocks);
    printf("Bit vector size                : %llu bits = %llu MB\n",
           disk_blocks, disk_blocks / 8 / MB);
    printf("FAT size (4 B per entry)       : %llu GB\n", disk_blocks * 4 / GB);
    printf("Linked free list (empty disk)  : 0 extra blocks, 4 B pointer in each free block\n");
    printf("Grouping (1023 addr per block) : %llu blocks to list every free block\n",
           (disk_blocks + 1022) / 1023);
    printf("Counting (empty disk)          : 1 entry (start 0, count %llu)\n", disk_blocks);
    return 0;
}
