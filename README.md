# Minidocker

A small container runtime in C, written to learn how Docker works under the hood.
Still early, built step by step.

## What works now
- `clone()` with new UTS, PID and mount namespaces
- custom hostname inside the container
- container process runs as PID 1
- `pivot_root` into a real rootfs (tested with Alpine minirootfs)
- private `/proc` mount, so `ps` shows only container processes
- runs any command inside the container

## Build
```
make
```

## Run
Get a rootfs first, for example Alpine:
```
mkdir rootfs
wget https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/x86_64/alpine-minirootfs-3.20.3-x86_64.tar.gz
tar -xzf alpine-minirootfs-*.tar.gz -C rootfs
```

Then run (needs root):
```
sudo ./build/program.out <hostname> <rootfs_path> <command> [args...]
sudo ./build/program.out mybox ./rootfs /bin/sh
```

## Todo
- cgroups (CPU and memory limits)
- network namespace + veth
- user namespace
- overlayfs for image layers
- proper cleanup and error handling