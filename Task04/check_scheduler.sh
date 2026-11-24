#!/usr/bin/env bash
# Task 4.4 - show the I/O scheduler and rotational flag of every block device
# Author: Shakthi M (192511197)
for d in /sys/block/*; do
  name=$(basename "$d")
  case "$name" in loop*|ram*|zram*) continue;; esac
  sched=$(cat "$d/queue/scheduler" 2>/dev/null)
  rot=$(cat "$d/queue/rotational" 2>/dev/null)
  [ "$rot" = "0" ] && kind="SSD/virtual (non-rotational)" || kind="HDD (rotational)"
  printf "%-8s scheduler: %-32s %s\n" "$name" "$sched" "$kind"
done
