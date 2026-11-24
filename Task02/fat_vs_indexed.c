/*
 * Task 2 - Alternative concept: FAT (linked, table in memory) vs indexed.
 * Measures the time to reach block k by following the FAT chain (O(k))
 * versus one look-up in an index array (O(1)).
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -O2 -o fat_vs_indexed fat_vs_indexed.c
 */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NBLK (1u << 20)                 /* 1,048,576 blocks = 4 GB file of 4 KB blocks */

static double now_ns(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

int main(void)
{
    unsigned *fat = malloc(sizeof(unsigned) * NBLK);       /* next-block table */
    unsigned *index = malloc(sizeof(unsigned) * NBLK);     /* index of data blocks */
    unsigned *perm = malloc(sizeof(unsigned) * NBLK);
    if (!fat || !index || !perm) return 1;

    /* scatter the file randomly over the disk, as after years of recording */
    for (unsigned i = 0; i < NBLK; i++) perm[i] = i;
    srand(42);
    for (unsigned i = NBLK - 1; i > 0; i--) {
        unsigned j = (unsigned)(((unsigned long long)rand() * rand()) % (i + 1));
        unsigned t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }
    for (unsigned i = 0; i < NBLK; i++) {
        index[i] = perm[i];
        fat[perm[i]] = (i + 1 < NBLK) ? perm[i + 1] : 0xFFFFFFFFu;
    }
    unsigned start = perm[0];

    unsigned ks[] = {10, 1000, 100000, 524288, NBLK - 1};
    printf("%-10s %-16s %-16s %-12s\n", "k", "FAT walk (us)", "Index (us)", "FAT lookups");
    volatile unsigned sink = 0;
    for (size_t t = 0; t < sizeof ks / sizeof ks[0]; t++) {
        unsigned k = ks[t];
        double t0 = now_ns();
        unsigned b = start;
        for (unsigned i = 0; i < k; i++) b = fat[b];
        double t1 = now_ns();
        unsigned c = index[k];
        double t2 = now_ns();
        sink ^= b ^ c;
        printf("%-10u %-16.2f %-16.3f %-12u %s\n", k, (t1 - t0) / 1e3, (t2 - t1) / 1e3, k,
               b == c ? "same block" : "MISMATCH");
    }
    printf("FAT memory for this 4 GB file : %.1f MB\n", NBLK * 4.0 / (1 << 20));
    printf("FAT memory for a 4 TB disk     : %.1f GB (2^30 entries x 4 B)\n", 4.0);
    (void)sink;
    free(fat); free(index); free(perm);
    return 0;
}
