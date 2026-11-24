#!/usr/bin/env bash
# Task 1.2 - Builds a small model of the improved student file-system layout
# inside ./college_fs (no root needed) and shows permissions, links and ACLs.
# Author: Shakthi M (192511197)   Course: CSA0415
set -e
ROOT=./college_fs
rm -rf "$ROOT"
mkdir -p "$ROOT"/home/{2024,2025}/{cse,ece}
mkdir -p "$ROOT"/projects/{cse-os-g07,ece-iot-g02}
mkdir -p "$ROOT"/shared/{courses,templates}

# two sample students: home = /home/<year>/<dept>/<regno>
for s in 2025/cse/192511197 2025/cse/192511150; do
  mkdir -p "$ROOT/home/$s"/{private,public,projects}
  echo "notes of $(basename $s)" > "$ROOT/home/$s/private/notes.txt"
  echo "resume of $(basename $s)" > "$ROOT/home/$s/public/resume.txt"
  chmod 700 "$ROOT/home/$s"            # owner only
  chmod 700 "$ROOT/home/$s/private"
  chmod 755 "$ROOT/home/$s/public"
done
chmod 711 "$ROOT"/home/2025/cse/192511197    # others may pass through to public/
chmod 2770 "$ROOT"/projects/cse-os-g07       # group rwx, setgid keeps group ownership
chmod 1777 "$ROOT"/shared/templates          # sticky bit: users delete only own files
echo "OS mini project report" > "$ROOT/projects/cse-os-g07/report.md"

# symbolic links from each member's home to the shared project folder (acyclic graph)
chmod 755 "$ROOT"/home/2025/cse/192511150
ln -s ../../../../../projects/cse-os-g07 "$ROOT/home/2025/cse/192511197/projects/cse-os-g07"
ln -s ../../../../../projects/cse-os-g07 "$ROOT/home/2025/cse/192511150/projects/cse-os-g07"
chmod 700 "$ROOT"/home/2025/cse/192511150

# ACL for the project (only if setfacl is installed)
if command -v setfacl >/dev/null 2>&1; then
  setfacl -m u:nobody:rx "$ROOT/projects/cse-os-g07" && \
  getfacl -p "$ROOT/projects/cse-os-g07"
else
  echo "(setfacl not installed: ACL step shown in report; install package 'acl')"
fi

echo "== Directory tree =="
find "$ROOT" -maxdepth 6 | sort | sed "s|$ROOT|college_fs|"
echo "== Permissions =="
ls -ld "$ROOT"/home/2025/cse/192511197 "$ROOT"/home/2025/cse/192511197/private \
       "$ROOT"/home/2025/cse/192511197/public "$ROOT"/projects/cse-os-g07 "$ROOT"/shared/templates
echo "== Link =="
ls -l "$ROOT"/home/2025/cse/192511150/projects 2>/dev/null || \
ls -l "$ROOT"/home/2025/cse/192511197/projects
