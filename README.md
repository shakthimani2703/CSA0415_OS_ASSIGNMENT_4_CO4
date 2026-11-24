# CSA0415 Operating Systems – Assignment 4 (CO4)
## Optimising File Storage and Disk Access

| | |
|---|---|
| **Name** | Shakthi M |
| **Register Number** | 192511197 |
| **Course Code** | CSA0415 – Operating Systems |
| **E-mail** | shakthimani2703@gmail.com |
| **Repository** | https://github.com/shakthimani2703/CSA0415_OS_ASSIGNMENT_4_CO4 |
| **Compiler** | gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| **OS** | Ubuntu 24.04.5 LTS, Linux kernel 6.x, ext4 root file system |
| **Compile command** | `gcc -std=c11 -Wall -Wextra -O2 file.c -o file -lm` |

All programs compile with **zero warnings** under `-std=c11 -Wall -Wextra`.

## Folder layout

```
Task01/  File-system design review   setup_student_fs.sh, search_cost.c
Task02/  File allocation             alloc.c (Complete the Code), frag_demo.c,
                                     calc_cost.c, fat_vs_indexed.c
Task03/  Modern file systems         inspect_fs.sh
Task04/  Disk scheduling             disk_sched.c (Complete the Code), all_algos.c,
                                     sched_sim.c, sstf_starvation.c, check_scheduler.sh
Task05/  Distributed storage         storage_model.c
Task06/  Design challenge            video_sim.c, video_read.fio
TaskXX/output/                       saved output of every program
```

## How to run

### Everything at once (Linux / WSL / VS Code terminal)
```bash
make            # builds all C programs
./run_all.sh    # runs every task and saves output to TaskXX/output/
make clean      # removes binaries
```

### One program at a time
```bash
cd Task02
gcc -std=c11 -Wall -Wextra -o alloc alloc.c
./alloc
```
In VS Code, open any `.c` file and press **Ctrl+Shift+B** (task "build active C file" in `.vscode/tasks.json`).

> Windows users: run inside WSL (Ubuntu). The shell scripts in Task01, Task03 and Task04 need Linux
> (`/sys/block`, `mkfs.ext4`, `dumpe2fs`, `tune2fs`). `fat_vs_indexed.c` uses `clock_gettime` (POSIX).

## Results summary

| Task | Program | Result |
|---|---|---|
| 1 | `search_cost` | Flat directory: 120,000 comparisons on average; tree: 167 (≈719× faster) |
| 1 | `setup_student_fs.sh` | Builds `/home/<year>/<dept>/<regno>` tree with 700/711/2770/1777 permissions and symlinks to a shared project folder |
| 2 | `alloc` | Output matches the expected output exactly (E contiguous at 19; indexed: index block 6) |
| 2 | `frag_demo` | 14-block file fails contiguously with 21 free blocks (largest hole 13); indexed allocation succeeds |
| 2 | `calc_cost` | 4 GB file: contiguous 1 read, linked 1,048,576 reads, inode 4 reads; max inode file ≈ 4 TB; bit vector for 4 TB = 128 MB |
| 2 | `fat_vs_indexed` | Reaching block 1,048,575: FAT walk ≈ 28 ms vs index ≈ 0.4 µs |
| 3 | `inspect_fs.sh` | Root fs ext4, 4096-byte blocks, 16,777,216 inodes; test ext4 image shows journal + extents + metadata_csum |
| 4 | `disk_sched` | Matches expected: FCFS 640, SSTF 236, SCAN 236, C-SCAN 183 (+199) |
| 4 | `all_algos` | LOOK 208, C-LOOK 153 (+169 jump) |
| 4 | `sched_sim` | Heavy load: FCFS max wait 26.7 s; LOOK lowest max wait 76 ms; edge/middle wait ratio SCAN 1.34 vs C-SCAN 1.18 |
| 4 | `sstf_starvation` | Request at 183 waits 4,020 ms under SSTF vs 23 ms under LOOK |
| 4 | `check_scheduler.sh` | Disk `vda` uses `mq-deadline` |
| 5 | `storage_model` | NFS saturates at ~500 users; 12-node DFS (RF 3) serves 3,000 users at 13 ms |
| 6 | `video_sim` | Design B (SSD + XFS extents + C-LOOK): 27 ms average start, 161 starts/s per HDD vs Design A 608 ms, 15 starts/s |

## Commit plan (one commit per task)
```bash
git init
git remote add origin https://github.com/shakthimani2703/CSA0415_OS_ASSIGNMENT_4_CO4.git
git add README.md Makefile run_all.sh .gitignore .vscode && git commit -m "Project setup"
git add Task01 && git commit -m "Task 1: file-system design review"
git add Task02 && git commit -m "Task 2: file allocation, complete the code"
git add Task03 && git commit -m "Task 3: modern file systems inspection"
git add Task04 && git commit -m "Task 4: disk scheduling, complete the code, simulation"
git add Task05 && git commit -m "Task 5: distributed storage model"
git add Task06 && git commit -m "Task 6: video storage design simulation"
git branch -M main && git push -u origin main
```
