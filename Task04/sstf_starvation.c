/*
 * Task 4 - Starvation demo: request at cylinder 183 while a stream of
 * requests near cylinder 60 arrives every millisecond.
 * Service = 0.1 ms per cylinder + 2 ms, so the stream keeps the queue
 * near 60 permanently busy. The stream stops after STREAM_MS.
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -o sstf_starvation sstf_starvation.c
 */
#include <stdio.h>
#include <stdlib.h>

#define STREAM_MS 2000          /* stream of requests for 2 seconds */
#define MAXQ (STREAM_MS + 16)

typedef struct { double arrive; int cyl; int id; } Req;

static double run(int alg, int *served_before)
{
    static Req all[MAXQ];
    int n = 0;
    all[n++] = (Req){0.0, 183, 1};                 /* the victim request */
    srand(7);
    for (int t = 0; t < STREAM_MS; t++)            /* stream near cylinder 60 */
        all[n++] = (Req){(double)t, 55 + rand() % 11, 0};

    static Req q[MAXQ];
    int nq = 0, next = 0, head = 53, dir = 1, count = 0;
    double now = 0;
    for (;;) {
        while (next < n && all[next].arrive <= now) q[nq++] = all[next++];
        if (nq == 0) { if (next >= n) break; now = all[next].arrive; continue; }
        int pick = 0;
        if (alg == 1) {                           /* SSTF */
            for (int i = 1; i < nq; i++)
                if (abs(q[i].cyl - head) < abs(q[pick].cyl - head)) pick = i;
        } else if (alg == 2) {                    /* LOOK (elevator) */
            pick = -1;
            for (int pass = 0; pass < 2 && pick < 0; pass++) {
                for (int i = 0; i < nq; i++) {
                    int d = (q[i].cyl - head) * dir;
                    if (d >= 0 && (pick < 0 || d < (q[pick].cyl - head) * dir)) pick = i;
                }
                if (pick < 0) dir = -dir;
            }
        }                                         /* alg 0 = FCFS: pick 0 */
        Req r = q[pick];
        for (int i = pick + 1; i < nq; i++) q[i - 1] = q[i];
        nq--;
        now += abs(r.cyl - head) * 0.1 + 2.0;
        head = r.cyl;
        if (r.id == 1) { *served_before = count; return now; }
        count++;
    }
    return -1;
}

int main(void)
{
    const char *nm[] = {"FCFS", "SSTF", "LOOK"};
    printf("Stream: 1 request per ms near cylinder 55-65 for %d ms; victim at 183 arrives at t=0\n",
           STREAM_MS);
    printf("%-6s %22s %22s\n", "Alg", "Victim served at (ms)", "Requests served before");
    for (int a = 0; a < 3; a++) {
        int before = 0;
        double t = run(a, &before);
        printf("%-6s %22.1f %22d\n", nm[a], t, before);
    }
    return 0;
}
