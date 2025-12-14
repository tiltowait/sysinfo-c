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
/\,-'''''-,/\    Memory:    12.3G / 16G (3.7G Avail)
\_)       (_/    Disk:      45.2G / 100.0G (45.2%)
|           |    ARC:       8.5G Total, 4.2G MFU, 3.8G MRU (49.4%)
|           |    IPv4:      [REDACTED]
 ;         ;     IPv6:      [REDACTED]
  '-_____-'      Uptime:    5d 12h 34m
```

## Limitations

This tool has no customization and hardcodes the network interface to `igb0`. To use a different interface, change the relevant line in `src/sysinfo.c`.
