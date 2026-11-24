/*
 * Task 4.1 + Alternative concept - order of service and total head movement
 * for FCFS, SSTF, SCAN, C-SCAN, LOOK and C-LOOK on the given queue.
 * SCAN/LOOK move towards 0 first; C-SCAN/C-LOOK move towards 199 first.
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -o all_algos all_algos.c
 */
#include <stdio.h>
#include <stdlib.h>

#define N 8
#define MAXCYL 199

static const int req[N] = {98, 183, 37, 122, 14, 124, 65, 67};
static const int HEAD = 53;

static int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

/* prints the path; 'jumps' marks positions reached by a return jump */
static void report(const char *name, const int path[], int n, const int jump[])
{
    int pos = HEAD, total = 0, jmp = 0;
    printf("%-7s %d", name, HEAD);
    for (int i = 0; i < n; i++) {
        int d = abs(path[i] - pos);
        if (jump[i]) { jmp += d; printf(" =>%d", path[i]); }
        else { total += d; printf(" %d", path[i]); }
        pos = path[i];
    }
    if (jmp) printf("  | seek %d + jump %d = %d\n", total, jmp, total + jmp);
    else     printf("  | total %d\n", total);
}

int main(void)
{
    int path[N + 3], jump[N + 3] = {0}, n, s[N];
    for (int i = 0; i < N; i++) s[i] = req[i];
    qsort(s, N, sizeof s[0], cmp);

    /* FCFS */
    for (int i = 0; i < N; i++) path[i] = req[i];
    report("FCFS", path, N, jump);

    /* SSTF */
    int done[N] = {0}, pos = HEAD;
    for (int k = 0; k < N; k++) {
        int best = -1;
        for (int i = 0; i < N; i++)
            if (!done[i] && (best < 0 || abs(req[i] - pos) < abs(req[best] - pos))) best = i;
        done[best] = 1; path[k] = req[best]; pos = req[best];
    }
    report("SSTF", path, N, jump);

    /* SCAN towards 0 */
    n = 0;
    for (int i = N - 1; i >= 0; i--) if (s[i] < HEAD) path[n++] = s[i];
    path[n++] = 0;
    for (int i = 0; i < N; i++) if (s[i] > HEAD) path[n++] = s[i];
    report("SCAN", path, n, jump);

    /* LOOK towards 0 */
    n = 0;
    for (int i = N - 1; i >= 0; i--) if (s[i] < HEAD) path[n++] = s[i];
    for (int i = 0; i < N; i++) if (s[i] > HEAD) path[n++] = s[i];
    report("LOOK", path, n, jump);

    /* C-SCAN towards 199, jump to 0 */
    n = 0;
    for (int i = 0; i < N; i++) if (s[i] > HEAD) path[n++] = s[i];
    path[n++] = MAXCYL;
    path[n] = 0; jump[n] = 1; n++;
    for (int i = 0; i < N; i++) if (s[i] < HEAD) path[n++] = s[i];
    report("C-SCAN", path, n, jump);
    for (int i = 0; i < N + 3; i++) jump[i] = 0;

    /* C-LOOK towards 199, jump to the lowest request */
    n = 0;
    for (int i = 0; i < N; i++) if (s[i] > HEAD) path[n++] = s[i];
    int first = 1;
    for (int i = 0; i < N; i++) if (s[i] < HEAD) { jump[n] = first; first = 0; path[n++] = s[i]; }
    report("C-LOOK", path, n, jump);
    return 0;
}
