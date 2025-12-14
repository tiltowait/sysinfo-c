# sysinfo

A lightweight FreeBSD system information display utility that shows the following:

* Memory usage
* Disk usage
* ZFS ARC statistics
* Network addresses
* Uptime

## Motivation

This is a re-implementation of [my original sysinfo](https://github.com/tiltowait/sysinfo). While that version does the job, I became mildly obsessed with avoiding shelling out to the system's `zfs` commands. This version, written in C, uses the OpenZFS library and achieves a ~40% runtime reduction on a Xeon E3-1270v6.

As with the Go-based `sysinfo`, this project exists because [fastfetch's](https://github.com/fastfetch-cli/fastfetch) calculations are off for FreeBSD. (See [the other readme](https://github.com/tiltowait/sysinfo) for more info)

## Requirements

- FreeBSD (tested on 14.3-RELEASE)

## Sample output

```
/\,-'''''-,/\   Memory: 16.6G / 64G (47.4G Avail)
\_)       (_/   Disk:   58.7G / 418.0G (14.1%)
|           |   ARC:    41.3G Total, 33.4G MFU, 4.7G MRU (80.8%)
|           |   IPv6:   [REDACTED]
 ;         ;    IPv4:   [REDACTED]
  '-_____-'     Uptime: 52d 12h 46m
```

**Note:** Memory usage intentionally excludes the ARC. Because of this, `sysinfo` will show different memory usage than `top`. [Read this blog post for more details.](https://blog.tiltowait.dev/the-mystery-of-the-disappearing-disk-space/)

## Limitations

This tool has no customization and hardcodes the network interface to `igb0`. To use a different interface, change the relevant line in `src/sysinfo.c`.
