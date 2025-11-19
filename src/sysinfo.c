/*
 * sysinfo.c - FreeBSD system information display utility
 *
 * Displays memory, disk, ZFS ARC stats, network addresses, and uptime
 * with a minimal ASCII BSD daemon mascot.
 */

#include "zfs.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* ANSI color codes */
#define RED_BOLD "\033[1;31m"
#define RESET "\033[0m"

/*
 * =====================================
 * Beastie ASCII art print functionality
 * =====================================
 */

// clang-format off
static const char *beastie_lines[] = {
    "/\\,-'''''-,/\\",
    "\\_)       (_/",
    "|           |",
    "|           |",
    " ;         ; ",
    "  '-_____-' "
};
// clang-format on
#define BEASTIE_LINES 6

/* BeastiePrinter structure */
typedef struct {
  int index;
} BeastiePrinter;

/* Initialize a BeastiePrinter */
static void beastie_init(BeastiePrinter *bp) { bp->index = 0; }

/* Get the next beastie line */
static const char *beastie_next(BeastiePrinter *bp) {
  if (bp->index >= BEASTIE_LINES) {
    return "            ";
  }
  return beastie_lines[bp->index++];
}

/* Print a line with beastie, label, and value */
static void beastie_print_line(BeastiePrinter *bp, const char *label,
                               const char *value) {
  printf("%s%s%s\t%s%s:%s\t%s\n", RED_BOLD, beastie_next(bp), RESET, RED_BOLD,
         label, RESET, value);
}

/*
 * ======================
 * Helpers and formatters
 * ======================
 */

/* Convert bytes to GB as a double */
static double bytes_to_gb(uint64_t bytes) {
  return (double)bytes / 1024.0 / 1024.0 / 1024.0;
}

/* Format bytes as GB with 1 decimal place */
static void format_gb(char *buf, size_t bufsize, uint64_t bytes) {
  snprintf(buf, bufsize, "%.1fG", bytes_to_gb(bytes));
}

/* Format duration as days, hours, minutes */
static void format_duration(char *buf, size_t bufsize, time_t seconds) {
  int days = seconds / 86400;
  int hours = (seconds % 86400) / 3600;
  int minutes = (seconds % 3600) / 60;

  if (days > 0) {
    snprintf(buf, bufsize, "%dd %dh %dm", days, hours, minutes);
  } else {
    snprintf(buf, bufsize, "%dh %dm", hours, minutes);
  }
}

/*
 * =============================
 * sysctl wrappers and stats calculators
 * =============================
 */

/* Get a uint64 sysctl value */
static int sysctl_uint64(const char *name, uint64_t *value) {
  size_t len = sizeof(*value);
  if (sysctlbyname(name, value, &len, NULL, 0) == -1) {
    return -1;
  }
  return 0;
}

/* Get a uint32 sysctl value */
static int sysctl_uint32(const char *name, uint32_t *value) {
  size_t len = sizeof(*value);
  if (sysctlbyname(name, value, &len, NULL, 0) == -1) {
    return -1;
  }
  return 0;
}

/* Get a timeval sysctl value */
static int sysctl_timeval(const char *name, struct timeval *tv) {
  size_t len = sizeof(*tv);
  if (sysctlbyname(name, tv, &len, NULL, 0) == -1) {
    return -1;
  }
  return 0;
}

/* Get uptime */
static int get_uptime(char *buf, size_t bufsize) {
  struct timeval boottime;
  if (sysctl_timeval("kern.boottime", &boottime) == -1) {
    return -1;
  }

  /* No error check. If the clock is broken, the user has bigger problems. */
  time_t now = time(NULL);
  time_t uptime = now - boottime.tv_sec;

  format_duration(buf, bufsize, uptime);
  return 0;
}

/* Get memory statistics */
static int get_memory(char *buf, size_t bufsize) {
  uint32_t pagesize;
  uint64_t physmem, arcsize, free_bytes;
  uint32_t free_pages;

  /*
   * Though it's extremely unlikely any of these sysctls will fail, we check
   * them all anyway, for completeness, and because Go has ruined me.
   */
  if (sysctl_uint32("hw.pagesize", &pagesize) == -1) {
    return -1;
  }
  if (sysctl_uint64("hw.realmem", &physmem) == -1) {
    return -1;
  }
  if (sysctl_uint64("kstat.zfs.misc.arcstats.size", &arcsize) == -1) {
    return -1;
  }
  if (sysctl_uint32("vm.stats.vm.v_free_count", &free_pages) == -1) {
    return -1;
  }

  free_bytes = (uint64_t)free_pages * pagesize;
  uint64_t available = free_bytes + arcsize;
  uint64_t used = physmem - available;

  char used_str[32], avail_str[32];
  format_gb(used_str, sizeof(used_str), used);
  format_gb(avail_str, sizeof(avail_str), available);

  snprintf(buf, bufsize, "%s / %.0fG (%s Avail)", used_str,
           bytes_to_gb(physmem), avail_str);
  return 0;
}

/* Get disk usage */
static int get_disk_usage(char *buf, size_t bufsize) {
  struct statfs stat;
  if (statfs("/", &stat) == -1) {
    return -1;
  }

  uint64_t total_bytes;

  /* It's FreeBSD, so we'll assume ZFS but fall back to UFS if that fails */
  if (get_zpool_size(&total_bytes) == -1) {
    total_bytes = (uint64_t)stat.f_blocks * stat.f_bsize;
  }

  uint64_t free_bytes = (uint64_t)stat.f_bfree * stat.f_bsize;
  uint64_t used_bytes = total_bytes - free_bytes;

  char used_str[32], total_str[32];
  format_gb(used_str, sizeof(used_str), used_bytes);
  format_gb(total_str, sizeof(total_str), total_bytes);

  /*
   * Technically, this might divide by zero. But in such a case, the system is
   * so catastrophically broken that SIGFPE is practically a feature.
   */
  double used_pct = (double)used_bytes / (double)total_bytes * 100.0;

  snprintf(buf, bufsize, "%s / %s (%.1f%%)", used_str, total_str, used_pct);
  return 0;
}

/* Get ZFS ARC statistics */
static int get_arc(char *buf, size_t bufsize) {
  uint64_t mfu_size, mru_size, arc_size;

  if (sysctl_uint64("kstat.zfs.misc.arcstats.mfu_size", &mfu_size) == -1) {
    return -1;
  }
  if (sysctl_uint64("kstat.zfs.misc.arcstats.mru_size", &mru_size) == -1) {
    return -1;
  }
  if (sysctl_uint64("kstat.zfs.misc.arcstats.size", &arc_size) == -1) {
    return -1;
  }

  char mfu_str[32], mru_str[32], arc_str[32];
  format_gb(mfu_str, sizeof(mfu_str), mfu_size);
  format_gb(mru_str, sizeof(mru_str), mru_size);
  format_gb(arc_str, sizeof(arc_str), arc_size);

  /*
   * If ZFS is in use, ARC can't ever be 0. If sysctl gave us 0 anyway, the
   * system is borked, and the user deserves SIGFPE.
   */
  double cache_pct = (double)mfu_size / (double)arc_size * 100.0;

  snprintf(buf, bufsize, "%s Total, %s MFU, %s MRU (%.1f%%)", arc_str, mfu_str,
           mru_str, cache_pct);
  return 0;
}

/* Holds interface address and IP type */
typedef struct {
  char type[8];
  char addr[INET6_ADDRSTRLEN];
} InterfaceAddr;

/* Get interface addresses */
static int get_interface_addrs(const char *ifname, InterfaceAddr *addrs,
                               int max_addrs) {
  struct ifaddrs *ifaddr, *ifa;
  int count = 0;

  if (getifaddrs(&ifaddr) == -1) {
    return -1;
  }

  for (ifa = ifaddr; ifa != NULL && count < max_addrs; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }

    /* Check if this is the interface we want */
    if (strcmp(ifa->ifa_name, ifname) != 0) {
      continue;
    }

    int family = ifa->ifa_addr->sa_family;

    if (family == AF_INET) {
      struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
      inet_ntop(AF_INET, &sa->sin_addr, addrs[count].addr, INET6_ADDRSTRLEN);
      strcpy(addrs[count].type, "IPv4");
      count++;
    } else if (family == AF_INET6) {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ifa->ifa_addr;

      /* Skip link-local addresses */
      if (IN6_IS_ADDR_LINKLOCAL(&sa->sin6_addr)) {
        continue;
      }

      inet_ntop(AF_INET6, &sa->sin6_addr, addrs[count].addr, INET6_ADDRSTRLEN);
      strcpy(addrs[count].type, "IPv6");
      count++;
    }
  }

  freeifaddrs(ifaddr);
  return count;
}

int main(void) {
  BeastiePrinter printer;
  beastie_init(&printer);

  char buf[256];

  if (get_memory(buf, sizeof(buf)) == 0) {
    beastie_print_line(&printer, "Memory", buf);
  }

  if (get_disk_usage(buf, sizeof(buf)) == 0) {
    beastie_print_line(&printer, "Disk", buf);
  }

  if (get_arc(buf, sizeof(buf)) == 0) {
    beastie_print_line(&printer, "ARC", buf);
  }

  /* Change the interface to match yours */
  InterfaceAddr addrs[10];
  int addr_count = get_interface_addrs("igb0", addrs, 10);
  if (addr_count > 0) {
    for (int i = 0; i < addr_count; i++) {
      beastie_print_line(&printer, addrs[i].type, addrs[i].addr);
    }
  }

  if (get_uptime(buf, sizeof(buf)) == 0) {
    beastie_print_line(&printer, "Uptime", buf);
  }

  while (printer.index < BEASTIE_LINES) {
    beastie_print_line(&printer, "", "");
  }

  return 0;
}
