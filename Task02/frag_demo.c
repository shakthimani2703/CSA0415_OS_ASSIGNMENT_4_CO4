/*
 * Task 2 - External fragmentation demo (extension of alloc.c)
 * Shows a file that FAILS contiguously although enough blocks are free,
 * and succeeds with indexed allocation.
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -o frag_demo frag_demo.c
 */
#include <stdio.h>
#include <string.h>

#define BLOCKS 32
static char disk[BLOCKS];

static int alloc_contiguous(char file, int size)
{
    int run = 0;
    for (int b = 0; b < BLOCKS; b++) {
        run = (disk[b] == '.') ? run + 1 : 0;
        if (run == size) {
            for (int i = b - size + 1; i <= b; i++) disk[i] = file;
            return b - size + 1;
        }
    }
    return -1;
}

static int alloc_indexed(char file, int size, int index[])
{
    int got = 0, idx = -1;
    for (int b = 0; b < BLOCKS; b++) {
        if (disk[b] != '.') continue;
        if (idx < 0) idx = b; else index[got++] = b;
        if (got == size) break;
    }
    if (idx < 0 || got < size) return -1;
    disk[idx] = (char)(file + 32);
    for (int i = 0; i < size; i++) disk[index[i]] = file;
    return idx;
}

static void release(char file)
{
    for (int b = 0; b < BLOCKS; b++) if (disk[b] == file) disk[b] = '.';
}

static int free_blocks(void)
{
    int f = 0;
    for (int b = 0; b < BLOCKS; b++) if (disk[b] == '.') f++;
    return f;
}

static int largest_hole(void)
{
    int best = 0, run = 0;
    for (int b = 0; b < BLOCKS; b++) {
        run = (disk[b] == '.') ? run + 1 : 0;
        if (run > best) best = run;
    }
    return best;
}

int main(void)
{
    memset(disk, '.', BLOCKS);
    alloc_contiguous('A', 6); alloc_contiguous('B', 8);
    alloc_contiguous('C', 5); alloc_contiguous('D', 7);
    release('B'); release('D');
    printf("Disk          : %.32s\n", disk);
    printf("Free blocks   : %d, largest hole: %d\n", free_blocks(), largest_hole());
    printf("F (14 blocks) contiguous: start %d\n", alloc_contiguous('F', 14));
    int index[20];
    int ib = alloc_indexed('F', 14, index);
    printf("F (14 blocks) indexed   : index block %d, data blocks", ib);
    for (int i = 0; i < 14; i++) printf(" %d", index[i]);
    printf("\nDisk          : %.32s\n", disk);
    return 0;
}
