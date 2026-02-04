// user/spinlock_demo.c
// Demonstration of spinlock behavior with priority inheritance

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("\n");
  printf("================================================================\n");
  printf("         SPINLOCK PRIORITY INHERITANCE TEST\n");
  printf("================================================================\n");
  printf("\n");
  
  printf("What are SPINLOCKS?\n");
  printf("-------------------\n");
  printf("  • Lock type that uses BUSY-WAITING (spinning)\n");
  printf("  • When a process wants a held lock, it SPINS in a loop\n");
  printf("  • Process keeps checking: while(locked) { keep_trying; }\n");
  printf("  • Uses CPU while waiting (doesn't yield/sleep)\n");
  printf("  • Fast for SHORT wait times\n");
  printf("  • Your xv6 implementation uses SPINLOCKS\n");
  printf("\n");
  
  printf("Why Priority Inheritance with Spinlocks?\n");
  printf("-----------------------------------------\n");
  printf("  Problem: HIGH spins waiting for LOW's lock\n");
  printf("           MEDIUM preempts LOW (higher priority than LOW)\n");
  printf("           HIGH wastes CPU spinning while MEDIUM runs\n");
  printf("           This is PRIORITY INVERSION!\n");
  printf("\n");
  printf("  Solution: When HIGH spins waiting for LOW's lock:\n");
  printf("            → Boost LOW to HIGH's priority\n");
  printf("            → LOW preempts MEDIUM and finishes fast\n");
  printf("            → HIGH stops spinning and gets the lock\n");
  printf("            → Restore LOW's original priority\n");
  printf("\n");
  
  printf("================================================================\n");
  printf("                     RUNNING TEST\n");
  printf("================================================================\n");
  printf("\n");
  
  int pid_parent = getpid();
  
  // LOW priority process - holds spinlock
  if(fork() == 0) {
    setpriority(10);
    int pid = getpid();
    
    printf("[PID=%d][LOW-10][SPINLOCK] Acquiring spinlock...\n", pid);
    test_acquire();
    
    printf("[PID=%d][LOW-10][SPINLOCK] Got lock! Spinning in critical section\n", pid);
    printf("[PID=%d][LOW-10][SPINLOCK] Current priority: %d\n", pid, getpriority());
    
    // Long work while holding spinlock
    cpu_work(100000000);
    
    printf("[PID=%d][LOW-10][SPINLOCK] Done working (priority=%d)\n", pid, getpriority());
    printf("[PID=%d][LOW-10][SPINLOCK] Releasing spinlock\n", pid);
    
    test_release();
    
    printf("[PID=%d][LOW-10][SPINLOCK] Released (priority=%d)\n", pid, getpriority());
    exit(0);
  }
  
  // Wait for LOW to acquire lock
  for(int i = 0; i < 15000000; i++);
  
  // MEDIUM priority process - doesn't need lock
  if(fork() == 0) {
    setpriority(5);
    int pid = getpid();
    
    printf("[PID=%d][MED-5][CPU] Doing CPU work (no spinlock needed)\n", pid);
    
    cpu_work(50000000);
    
    printf("[PID=%d][MED-5][CPU] Work complete\n", pid);
    exit(0);
  }
  
  // Wait for MEDIUM to start
  for(int i = 0; i < 5000000; i++);
  
  // HIGH priority process - needs the spinlock
  if(fork() == 0) {
    setpriority(1);
    int pid = getpid();
    
    printf("[PID=%d][HIGH-1][SPINLOCK] Need the spinlock!\n", pid);
    printf("[PID=%d][HIGH-1][SPINLOCK] Starting to SPIN waiting for lock...\n", pid);
    printf("[PID=%d][HIGH-1][SPINLOCK] *** Priority inheritance should trigger! ***\n", pid);
    
    int start = uptime();
    
    test_acquire();
    
    int elapsed = uptime() - start;
    
    printf("[PID=%d][HIGH-1][SPINLOCK] Got lock! (spun for %d ticks)\n", pid, elapsed);
    
    cpu_work(5000000);
    
    test_release();
    
    printf("[PID=%d][HIGH-1][SPINLOCK] Released lock\n", pid);
    exit(0);
  }
  
  // Wait for all children
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n");
  printf("================================================================\n");
  printf("                     TEST COMPLETE\n");
  printf("================================================================\n");
  printf("\n");
  
  printf("Key Observations:\n");
  printf("-----------------\n");
  printf("  1. HIGH process SPINS (busy-waits) for the lock\n");
  printf("  2. During spinning, priority inheritance boosts LOW\n");
  printf("  3. LOW finishes quickly at HIGH priority\n");
  printf("  4. HIGH stops spinning and acquires lock\n");
  printf("\n");
  
  printf("Look for these kernel messages:\n");
  printf("  [KERNEL][BOOST] Process X boosted: 10 → 1\n");
  printf("  [KERNEL][RESTORE] Process X restored: 1 → 10\n");
  printf("\n");
  
  exit(0);
}
