/*
 * Task 5 - Alternative concept: one large NFS server versus a replicated
 * distributed file system (HDFS/Ceph style, replication factor 3).
 *
 * Response time uses an M/M/c-style approximation (M/M/1 per server):
 *   R = S / (1 - rho),  rho = lambda * S / servers
 * Each user issues 0.5 file operations per second; S = 4 ms per op.
 * Reads can go to any replica; writes go to all 3 replicas (pipeline).
 * Availability: each server is up 99 % of the time (independent failures).
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -o storage_model storage_model.c -lm
 */
#include <stdio.h>
#include <math.h>

static double resp(double lambda, double S, int servers)
{
    double rho = lambda * S / servers;
    if (rho >= 1.0) return INFINITY;
    return S / (1.0 - rho);
}

int main(void)
{
    const double S = 0.004, ops_per_user = 0.5, read_frac = 0.8;
    const int nodes = 12, rf = 3;
    int users[] = {50, 200, 400, 480, 1000, 2000, 3000, 4000};
    printf("Response time (ms); NFS = 1 server, DFS = %d nodes, RF = %d, 80%% reads\n", nodes, rf);
    printf("%-8s %12s %12s %12s %12s\n", "Users", "NFS read", "NFS write", "DFS read", "DFS write");
    for (size_t i = 0; i < sizeof users / sizeof users[0]; i++) {
        double lam = users[i] * ops_per_user;
        double nfs = resp(lam, S, 1);
        /* DFS load: reads x1, writes x3 spread over all nodes */
        double dfs_load = lam * (read_frac + (1 - read_frac) * rf);
        double dr = resp(dfs_load, S, nodes);
        double dw = isinf(dr) ? INFINITY : dr * 1.3;   /* pipeline to 3 replicas */
        printf("%-8d %12.2f %12.2f %12.2f %12.2f\n", users[i], nfs * 1000, nfs * 1000,
               dr * 1000, dw * 1000);
    }
    double a = 0.99;
    printf("\nAvailability of one block: NFS (1 copy) = %.4f %%, DFS (3 replicas) = %.6f %%\n",
           a * 100, (1 - pow(1 - a, rf)) * 100);
    printf("Expected downtime per year: NFS = %.1f h, DFS = %.2f min\n",
           (1 - a) * 8760, pow(1 - a, rf) * 8760 * 60);
    double data_tb = 100;
    printf("Raw storage for %.0f TB of user data: NFS = %.0f TB (+backup), DFS RF3 = %.0f TB,"
           " erasure code 8+3 = %.1f TB\n", data_tb, data_tb, data_tb * rf, data_tb * 11.0 / 8);
    return 0;
}
