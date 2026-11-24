/*
 * Task 6 Stage 5 - Simulation of two storage designs for a lecture-video
 * streaming service (200,000 students, 3,000 videos of 300 MB - 2 GB).
 *
 * Each event = a student starts playing a video at a random minute.
 * The server must read the metadata and the first 4 MB play-out buffer.
 *
 * Design A : HDD only, ext4-style 4 KB blocks with indirect block maps,
 *            files fragmented in 256 KB runs (concurrent uploads),
 *            64 GB RAM page cache, FCFS ("none") disk queue.
 * Design B : SSD cache tier (2 TB) + HDD capacity tier, XFS-style extents
 *            (files laid out in 64 MB extents, 1 MB stripe unit),
 *            64 GB RAM page cache, C-LOOK ordering (mq-deadline batches).
 *
 * HDD model: 10,000 cylinders, seek(d) = 1 + 9*sqrt(d/10000) ms (d > 0),
 *            7200 rpm -> 4.17 ms average rotation, 200 MB/s transfer.
 * SSD model: 0.08 ms per read + 2,000 MB/s transfer.
 * Warm-up : the caches are first warmed with 200,000 plays that are not counted.
 * Popularity: 70 % of plays go to the 300 videos of the current week,
 *            Zipf(s = 1.0) inside each group.
 *
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -O2 -o video_sim video_sim.c -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define NVIDEO   3000
#define NEW      300                  /* videos uploaded this week */
#define NPLAYS   200000
#define BATCH    32                   /* requests queued together */
#define CHUNK_MB 4
#define CYL      10000

typedef struct { int size_mb; int cyl0; } Video;
static Video vid[NVIDEO];
static double zipf_new[NEW], zipf_old[NVIDEO - NEW];

/* ---------- reproducible random numbers ---------- */
static unsigned long long rs = 88172645463325252ULL;
static double urand(void)
{
    rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17;
    return (double)(rs >> 11) / 9007199254740992.0;
}

static void build_zipf(double *cdf, int n)
{
    double s = 0;
    for (int i = 0; i < n; i++) { s += 1.0 / (i + 1); cdf[i] = s; }
    for (int i = 0; i < n; i++) cdf[i] /= s;
}
static int draw(const double *cdf, int n)
{
    double u = urand();
    int lo = 0, hi = n - 1;
    while (lo < hi) { int m = (lo + hi) / 2; if (cdf[m] < u) lo = m + 1; else hi = m; }
    return lo;
}

/* ---------- LRU cache of keys (hash + doubly linked list) ---------- */
typedef struct { long long *key; int *prev, *next, *hnext, *htab; int cap, n, head, tail, hsz, freep; } LRU;

static void lru_init(LRU *c, int cap)
{
    c->cap = cap; c->n = 0; c->head = c->tail = -1; c->hsz = cap * 2 + 1;
    c->key = malloc(sizeof(long long) * cap); c->prev = malloc(sizeof(int) * cap);
    c->next = malloc(sizeof(int) * cap); c->hnext = malloc(sizeof(int) * cap);
    c->htab = malloc(sizeof(int) * c->hsz);
    for (int i = 0; i < c->hsz; i++) c->htab[i] = -1;
}
static void lru_free(LRU *c) { free(c->key); free(c->prev); free(c->next); free(c->hnext); free(c->htab); }
static int hidx(LRU *c, long long k) { return (int)((unsigned long long)(k * 11400714819323198485ULL) % (unsigned)c->hsz); }
static void unlink_node(LRU *c, int i)
{
    if (c->prev[i] >= 0) c->next[c->prev[i]] = c->next[i]; else c->head = c->next[i];
    if (c->next[i] >= 0) c->prev[c->next[i]] = c->prev[i]; else c->tail = c->prev[i];
}
static void push_front(LRU *c, int i)
{
    c->prev[i] = -1; c->next[i] = c->head;
    if (c->head >= 0) c->prev[c->head] = i;
    c->head = i; if (c->tail < 0) c->tail = i;
}
static void hremove(LRU *c, int i)
{
    int h = hidx(c, c->key[i]), *p = &c->htab[h];
    while (*p != i) p = &c->hnext[*p];
    *p = c->hnext[i];
}
/* returns 1 on hit; inserts on miss */
static int lru_access(LRU *c, long long k)
{
    int h = hidx(c, k);
    for (int i = c->htab[h]; i >= 0; i = c->hnext[i])
        if (c->key[i] == k) { unlink_node(c, i); push_front(c, i); return 1; }
    int slot;
    if (c->n < c->cap) slot = c->n++;
    else { slot = c->tail; unlink_node(c, slot); hremove(c, slot); }
    c->key[slot] = k; c->hnext[slot] = c->htab[h]; c->htab[h] = slot;
    push_front(c, slot);
    return 0;
}

/* ---------- device models ---------- */
static double seek_ms(int d) { return d == 0 ? 0.0 : 1.0 + 9.0 * sqrt((double)abs(d) / CYL); }
#define ROT_MS   4.17
#define HDD_MBPS 200.0
#define SSD_MS   0.08
#define SSD_MBPS 2000.0

typedef struct { int cyl; double mb; } Op;     /* one HDD access */

typedef struct {
    const char *name;
    int run_kb;            /* contiguous run length on HDD */
    int meta_reads;        /* metadata reads for a cold file (indirect / extent) */
    int ssd_tier;          /* 1 = SSD cache present */
    int clook;             /* 1 = C-LOOK ordering, 0 = FCFS */
} Design;

typedef struct { double avg_ms, p95_ms, hdd_ops, ram_hit, ssd_hit, hdd_busy_ms, starts_per_s; } Stats;

static int cmp_d(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }

static Stats run(const Design *D)
{
    LRU ram, ssd;
    lru_init(&ram, 64 * 1024 / CHUNK_MB);                 /* 64 GB RAM     */
    if (D->ssd_tier) lru_init(&ssd, 2 * 1024 * 1024 / CHUNK_MB);  /* 2 TB SSD */
    rs = 88172645463325252ULL;
    double *lat = malloc(sizeof(double) * NPLAYS);
    long long hdd_ops = 0, ram_hits = 0, ssd_hits = 0, chunks = 0;
    double hdd_busy = 0;
    int head = 0;

    static Op ops[BATCH * 64];
    static int owner[BATCH * 64];
    double ready[BATCH];

    int nb = NPLAYS / BATCH;
    for (int bb = 0; bb < 2 * nb; bb++) {        /* first half = cache warm-up */
        int warm = bb < nb, b = bb - nb;
        int nops = 0;
        for (int r = 0; r < BATCH; r++) {
            int v = (urand() < 0.70) ? draw(zipf_new, NEW) : NEW + draw(zipf_old, NVIDEO - NEW);
            int chunk = (int)(urand() * (vid[v].size_mb / CHUNK_MB));
            long long key = (long long)v * 100000 + chunk;
            long long mkey = -(long long)v - 1;            /* metadata key */
            ready[r] = 0.05;                               /* CPU + network stack */
            if (!warm) chunks++;
            /* metadata */
            if (!lru_access(&ram, mkey)) {
                for (int m = 0; m < D->meta_reads; m++) {
                    ops[nops] = (Op){vid[v].cyl0, 0.004}; owner[nops++] = r;
                }
            }
            /* data chunk */
            if (lru_access(&ram, key)) { if (!warm) ram_hits++; continue; }
            if (D->ssd_tier && lru_access(&ssd, key)) {
                if (!warm) ssd_hits++;
                ready[r] += SSD_MS + CHUNK_MB / SSD_MBPS * 1000; continue;
            }
            int runs = (CHUNK_MB * 1024) / D->run_kb; if (runs < 1) runs = 1;
            int base = vid[v].cyl0 + (int)((double)chunk * CHUNK_MB / vid[v].size_mb * 40);
            for (int k = 0; k < runs; k++) {
                int c = base + (D->run_kb < 1024 ? (int)(urand() * 400) - 200 : 0);
                if (c < 0) c = 0;
                if (c >= CYL) c = CYL - 1;
                ops[nops] = (Op){c, (double)CHUNK_MB / runs}; owner[nops++] = r;
            }
        }
        /* order the batch */
        if (D->clook) {
            /* C-LOOK: serve cylinders >= head ascending, then wrap to lowest */
            Op tmp[BATCH * 64]; int tow[BATCH * 64], idx[BATCH * 64];
            for (int i = 0; i < nops; i++) idx[i] = i;
            for (int i = 1; i < nops; i++) {               /* insertion sort by cylinder */
                int x = idx[i], j = i - 1;
                while (j >= 0 && ops[idx[j]].cyl > ops[x].cyl) { idx[j + 1] = idx[j]; j--; }
                idx[j + 1] = x;
            }
            int start = 0; while (start < nops && ops[idx[start]].cyl < head) start++;
            for (int i = 0; i < nops; i++) {
                int s = idx[(start + i) % nops];
                tmp[i] = ops[s]; tow[i] = owner[s];
            }
            memcpy(ops, tmp, sizeof(Op) * nops); memcpy(owner, tow, sizeof(int) * nops);
        }
        double t = 0;
        for (int i = 0; i < nops; i++) {
            t += seek_ms(ops[i].cyl - head) + ROT_MS + ops[i].mb / HDD_MBPS * 1000;
            head = ops[i].cyl;
            if (t > ready[owner[i]]) ready[owner[i]] = t;  /* request done after its last op */
        }
        if (warm) continue;
        hdd_ops += nops; hdd_busy += t;
        for (int r = 0; r < BATCH; r++) lat[b * BATCH + r] = ready[r];
    }
    int n = (NPLAYS / BATCH) * BATCH;
    double sum = 0; for (int i = 0; i < n; i++) sum += lat[i];
    qsort(lat, n, sizeof(double), cmp_d);
    Stats s;
    s.avg_ms = sum / n; s.p95_ms = lat[(int)(0.95 * n)];
    s.hdd_ops = (double)hdd_ops / n;
    s.ram_hit = 100.0 * ram_hits / chunks; s.ssd_hit = 100.0 * ssd_hits / chunks;
    s.hdd_busy_ms = hdd_busy / n;
    s.starts_per_s = 1000.0 / s.hdd_busy_ms;
    free(lat); lru_free(&ram); if (D->ssd_tier) lru_free(&ssd);
    return s;
}

int main(void)
{
    rs = 12345;
    long long total = 0;
    for (int i = 0; i < NVIDEO; i++) {
        vid[i].size_mb = 300 + (int)(urand() * 1748);
        vid[i].cyl0 = (int)(urand() * (CYL - 50));
        total += vid[i].size_mb;
    }
    build_zipf(zipf_new, NEW); build_zipf(zipf_old, NVIDEO - NEW);
    printf("Catalogue: %d videos, %.2f TB, average %.0f MB; %d play starts, batches of %d\n",
           NVIDEO, total / 1024.0 / 1024.0, (double)total / NVIDEO, NPLAYS, BATCH);

    Design d[] = {
        {"A: HDD-only ext4 4KB, FCFS", 256, 2, 0, 0},
        {"A': HDD-only ext4 4KB, C-LOOK", 256, 2, 0, 1},
        {"B': SSD+HDD XFS extents, FCFS", 65536, 1, 1, 0},
        {"B: SSD+HDD XFS extents, C-LOOK", 65536, 1, 1, 1},
    };
    printf("\n%-32s %9s %9s %9s %8s %8s %10s\n", "Design", "Avg(ms)", "P95(ms)",
           "HDDops", "RAM%", "SSD%", "Starts/s");
    for (int i = 0; i < 4; i++) {
        Stats s = run(&d[i]);
        printf("%-32s %9.1f %9.1f %9.2f %8.1f %8.1f %10.1f\n", d[i].name, s.avg_ms, s.p95_ms,
               s.hdd_ops, s.ram_hit, s.ssd_hit, s.starts_per_s);
    }
    printf("\nStarts/s = play starts one HDD can sustain (1000 / HDD busy ms per start)\n");
    return 0;
}
