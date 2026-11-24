# CSA0415 Operating Systems - Assignment 4 (CO4)
# Shakthi M - 192511197
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
LDLIBS  = -lm

BINS = Task01/search_cost Task02/alloc Task02/frag_demo Task02/calc_cost \
       Task02/fat_vs_indexed Task04/disk_sched Task04/all_algos Task04/sched_sim \
       Task04/sstf_starvation Task05/storage_model Task06/video_sim

all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

run: all
	./run_all.sh

clean:
	rm -f $(BINS) Task03/sample.txt Task03/ext4.img
	rm -rf Task01/college_fs

.PHONY: all run clean
