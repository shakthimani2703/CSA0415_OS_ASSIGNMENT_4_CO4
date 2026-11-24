/*
 * Task 1 - Search cost: one flat directory of 12,000 x 20 files versus
 * a tree /home/<year>/<dept>/<regno>. Counts directory-entry comparisons
 * for a linear directory scan (as in a simple linear-list directory).
 * Author : Shakthi M (192511197)
 * Build  : gcc -std=c11 -Wall -Wextra -o search_cost search_cost.c
 */
#include <stdio.h>

int main(void)
{
    const long students = 12000, files_per_student = 20;
    const long years = 4, depts = 10;
    long flat = students * files_per_student;
    long per_dept = students / (years * depts);
    printf("Flat directory entries        : %ld\n", flat);
    printf("Average comparisons (flat)    : %ld\n", flat / 2);
    printf("Tree levels: home -> year(%ld) -> dept(%ld) -> regno(%ld) -> file(%ld)\n",
           years, depts, per_dept, files_per_student);
    long tree = years / 2 + depts / 2 + per_dept / 2 + files_per_student / 2;
    printf("Average comparisons (tree)    : %ld\n", tree);
    printf("Speed-up                      : %.0fx\n", (double)(flat / 2) / tree);
    printf("With hashed/B-tree directories (ext4 htree): about log2(n) per level\n");
    return 0;
}
