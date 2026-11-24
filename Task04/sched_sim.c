/*
 * Task 4.2 - Simulation of FCFS, SSTF, SCAN, C-SCAN, LOOK and C-LOOK
 * with 10,000 random requests under light, medium and heavy load.
 *
 * Model : cylinders 0-199; requests arrive with exponential inter-arrival
 *         times; service time = 0.1 ms per cylinder moved + 2 ms
 *         (rotation + transfer).  The scheduler picks the next request
 *         from the pending queue whenever the head becomes free.
 * Output: average head movement per request (cylinders), average,
 *         maximum and standard deviation of waiting time (ms), and the
 *         average wait of edge (0-19, 180-199) vs middle (90-109) requests.
 * Author: Shakthi M (192511197)
 * Build : gcc -std=c11 -Wall -Wextra -O2 -o sched_sim sched_sim.c -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NREQ   10000
#define MAXCYL 199
#define SEEK_MS_PER_CYL 0.1
#define FIXED_MS 2.0

enum { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, NALG };
static const char *names[NALG] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};

typedef struct { double arrive; int cyl; } Req;

static Req reqs[NREQ];

static unsigned long long rng_state;
static double urand(void)                       /* xorshift64*, reproducible */
{
    rng_state ^= rng_state >> 12; rng_state ^= rng_state << 25; rng_state ^= rng_state >> 27;
    return (double)((rng_state * 2685821657736338717ULL) >> 11) / 9007199254740992.0;
}

static void make_workload(double mean_gap_ms, unsigned long long seed)
{
    rng_state = seed;
    double t = 0;
    for (int i = 0; i < NREQ; i++) {
        t += -mean_gap_ms * log(1.0 - urand());
        reqs[i].arrive = t;
        reqs[i].cyl = (int)(urand() * (MAXCYL + 1));
    }
}

typedef struct { double avg_move, avg_wait, max_wait, total_move, sd_wait, edge_wait, mid_wait; } Result;

static Result simulate(int alg)
{
    static int pend[NREQ];                     /* indices of pending requests */
    int np = 0, next = 0, served = 0, head = 53, dir = -1;  /* dir -1 = towards 0 */
    double now = 0, sum_wait = 0, max_wait = 0, sum_sq = 0, edge = 0, mid = 0;
    int nedge = 0, nmid = 0;
    long long moved = 0;

    while (served < NREQ) {
        while (next < NREQ && reqs[next].arrive <= now) pend[np++] = next++;
        if (np == 0) { now = reqs[next].arrive; continue; }

        int pick = -1;                          /* position in pend[] */
        int extra = 0;                          /* end-of-disk travel / jump */
        if (alg == FCFS) {
            pick = 0;
        } else if (alg == SSTF) {
            for (int i = 0; i < np; i++)
                if (pick < 0 || abs(reqs[pend[i]].cyl - head) < abs(reqs[pend[pick]].cyl - head))
                    pick = i;
        } else {
            int cdir = (alg == CSCAN || alg == CLOOK) ? +1 : dir;
            /* nearest request in the current direction (including at head) */
            for (int i = 0; i < np; i++) {
                int c = reqs[pend[i]].cyl, d = (c - head) * cdir;
                if (d >= 0 && (pick < 0 || d < (reqs[pend[pick]].cyl - head) * cdir)) pick = i;
            }
            if (pick < 0) {                     /* nothing ahead: turn or jump */
                if (alg == SCAN || alg == LOOK) {
                    if (alg == SCAN) { int end = dir < 0 ? 0 : MAXCYL; extra = abs(end - head); head = end; }
                    dir = -dir;
                    for (int i = 0; i < np; i++) {
                        int d = (reqs[pend[i]].cyl - head) * dir;
                        if (d >= 0 && (pick < 0 || d < (reqs[pend[pick]].cyl - head) * dir)) pick = i;
                    }
                } else {                        /* C-SCAN / C-LOOK: jump to low end */
                    int low = 0;
                    if (alg == CSCAN) { extra = (MAXCYL - head) + MAXCYL; head = 0; }
                    for (int i = 0; i < np; i++)
                        if (pick < 0 || reqs[pend[i]].cyl < reqs[pend[pick]].cyl) pick = i;
                    if (alg == CLOOK) { low = reqs[pend[pick]].cyl; extra = head - low; head = low; }
                    (void)low;
                }
            }
        }
        int id = pend[pick];
        int dist = abs(reqs[id].cyl - head) + extra;
        moved += dist;
        now += dist * SEEK_MS_PER_CYL + FIXED_MS;
        head = reqs[id].cyl;
        double w = now - reqs[id].arrive;
        sum_wait += w; sum_sq += w * w;
        if (reqs[id].cyl < 20 || reqs[id].cyl >= 180) { edge += w; nedge++; }
        else if (reqs[id].cyl >= 90 && reqs[id].cyl < 110) { mid += w; nmid++; }
        if (w > max_wait) max_wait = w;
        if (alg == FCFS) { for (int i = 1; i < np; i++) pend[i - 1] = pend[i]; np--; }
        else pend[pick] = pend[--np];
        served++;
    }
    double mean = sum_wait / NREQ;
    Result r = { (double)moved / NREQ, mean, max_wait, (double)moved,
                 sqrt(sum_sq / NREQ - mean * mean), edge / nedge, mid / nmid };
    return r;
}

int main(void)
{
    struct { const char *name; double gap; } loads[] = {
        {"Light  (1 req / 20 ms)", 20.0},
        {"Medium (1 req / 10 ms)", 10.0},
        {"Heavy  (1 req / 6 ms)",  6.0},
    };
    for (int l = 0; l < 3; l++) {
        make_workload(loads[l].gap, 2025 + l);
        printf("\n%s, %d requests, cylinders 0-%d (move in cylinders, times in ms)\n", loads[l].name, NREQ, MAXCYL);
        printf("%-7s %12s %12s %12s %12s %12s %12s\n", "Alg", "Avg move", "Avg wait", "Max wait",
               "Std dev", "Edge wait", "Mid wait");
        for (int a = 0; a < NALG; a++) {
            Result r = simulate(a);
            printf("%-7s %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f\n", names[a], r.avg_move,
                   r.avg_wait, r.max_wait, r.sd_wait, r.edge_wait, r.mid_wait);
        }
    }
    return 0;
}
