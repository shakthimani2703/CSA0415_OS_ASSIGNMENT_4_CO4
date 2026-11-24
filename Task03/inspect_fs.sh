#!/usr/bin/env bash
# Task 3.4 - Inspect a real file system: inodes, block size and free space.
# Creates a small ext4 image so tune2fs/dumpe2fs work without root, and also
# inspects the root file system of the machine.
# Author: Shakthi M (192511197)
echo "### uname / distribution"; uname -r; grep PRETTY_NAME /etc/os-release
echo; echo "### df -hT /"; df -hT /
echo; echo "### df -i /"; df -i /
echo "test data" > sample.txt
echo; echo "### stat sample.txt"; stat sample.txt
echo; echo "### ls -i"; ls -i sample.txt inspect_fs.sh
echo; echo "### stat -f . (file-system view)"; stat -f .
truncate -s 64M ext4.img
mkfs.ext4 -q -F -b 4096 -L CSA0415 ext4.img
echo; echo "### dumpe2fs -h ext4.img (selected lines)"
dumpe2fs -h ext4.img 2>/dev/null | grep -E "Filesystem volume name|Filesystem features|Inode count|Block count|Free blocks|Free inodes|Block size|Inode size|Inodes per group|Journal size|Default mount options"
echo; echo "### tune2fs -l ext4.img | grep -i journal"
tune2fs -l ext4.img | grep -iE "journal|has_journal" | head -5
rm -f ext4.img
