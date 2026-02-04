// user/sleeplock_test.c
// Test priority inheritance with sleeplocks (not spinlocks)

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Note: This requires sleeplock support in the kernel
// For now, this is a template showing how it would work

int
main(void)
{
  printf("\n");
  printf("================================================================\n");
  printf("         SLEEPLOCK PRIORITY INHERITANCE TEST\n");
  printf("================================================================\n");
  printf("\n");
  printf("Sleeplocks are different from spinlocks:\n");
  printf("  - Spinlocks: Busy-wait (spin) while waiting\n");
  printf("  - Sleeplocks: Process sleeps (yields CPU) while waiting\n");
  printf("\n");
  printf("Priority inheritance with sleeplocks:\n");
  printf("  1. LOW acquires sleeplock\n");
  printf("  2. HIGH tries to acquire, goes to sleep\n");
  printf("  3. LOW inherits HIGH's priority while holding lock\n");
  printf("  4. LOW runs at high priority, finishes quickly\n");
  printf("  5. HIGH wakes up and gets the lock\n");
  printf("\n");
  printf("================================================================\n");
  printf("\nNote: This test requires sleeplock support in kernel\n");
  printf("      Your current implementation uses spinlocks only\n");
  printf("\n");
  
  exit(0);
}
