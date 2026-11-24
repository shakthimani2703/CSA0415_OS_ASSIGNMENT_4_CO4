#!/usr/bin/env bash
# Builds every program and saves its output to TaskXX/output/
# Shakthi M - 192511197
set -e
cd "$(dirname "$0")"
make -s all
save() { mkdir -p "$1/output"; echo "\$ $2" > "$1/output/$3"; (cd "$1" && eval "$2") >> "$1/output/$3" 2>&1; cat "$1/output/$3"; echo; }
save Task01 "./search_cost"            search_cost.txt
save Task01 "./setup_student_fs.sh"    setup_student_fs.txt
save Task02 "./alloc"                  alloc.txt
save Task02 "./frag_demo"              frag_demo.txt
save Task02 "./calc_cost"              calc_cost.txt
save Task02 "./fat_vs_indexed"         fat_vs_indexed.txt
save Task03 "./inspect_fs.sh"          inspect_fs.txt
save Task04 "./disk_sched"             disk_sched.txt
save Task04 "./all_algos"              all_algos.txt
save Task04 "./sched_sim"              sched_sim.txt
save Task04 "./sstf_starvation"        sstf_starvation.txt
save Task04 "./check_scheduler.sh"     check_scheduler.txt
save Task05 "./storage_model"          storage_model.txt
save Task06 "./video_sim"              video_sim.txt
