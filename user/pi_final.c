// user/pi_final.c
// Clean priority inheritance test with clear output

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Synchronization flags
volatile int low_has_lock = 0;
volatile int medium_started = 0;
volatile int high_started = 0;

void
print_header(void)
{
  printf("\n");
  printf("========================================================\n");
  printf("         PRIORITY INHERITANCE DEMONSTRATION\n");
  printf("========================================================\n\n");
  printf("Three processes will demonstrate priority inheritance:\n");
  printf("  LOW    (Priority 10) - Acquires lock, does long work\n");
  printf("  MEDIUM (Priority 5)  - Does CPU work without lock\n");
  printf("  HIGH   (Priority 1)  - Tries to acquire same lock\n\n");
  printf("Expected: LOW will be boosted to priority 1 when HIGH waits\n");
  printf("========================================================\n\n");
}

void
low_priority_process(void)
{
  int mypid = getpid();
  
  // Set priority BEFORE doing anything
  setpriority(10);
  
  printf("[PID=%d][LOW] Starting with priority 10\n", mypid);
  printf("[PID=%d][LOW] Acquiring lock...\n", mypid);
  
  test_acquire();
  low_has_lock = 1;  // Signal that we have the lock
  
  printf("[PID=%d][LOW] Lock acquired! Starting long work...\n", mypid);
  printf("[PID=%d][LOW] My priority is now: %d\n", mypid, getpriority());
  
  // Do long work while holding the lock
  cpu_work(150000000);
  
  printf("[PID=%d][LOW] Work complete!\n", mypid);
  printf("[PID=%d][LOW] My priority before release: %d\n", mypid, getpriority());
  printf("[PID=%d][LOW] Releasing lock...\n", mypid);
  
  test_release();
  
  printf("[PID=%d][LOW] Lock released. My priority now: %d\n", mypid, getpriority());
  printf("[PID=%d][LOW] DONE\n\n", mypid);
}

void
medium_priority_process(void)
{
  int mypid = getpid();
  
  // Wait for LOW to acquire the lock first
  while(!low_has_lock) {
    // Busy wait
  }
  
  // Small delay to ensure LOW is working
  for(int i = 0; i < 10000000; i++);
  
  // Now set our priority
  setpriority(5);
  
  printf("[PID=%d][MEDIUM] Starting with priority 5\n", mypid);
  printf("[PID=%d][MEDIUM] Doing CPU work (no lock needed)...\n", mypid);
  
  medium_started = 1;
  
  // Do CPU work without needing the lock
  cpu_work(80000000);
  
  printf("[PID=%d][MEDIUM] Work complete!\n", mypid);
  printf("[PID=%d][MEDIUM] DONE\n\n", mypid);
}

void
high_priority_process(void)
{
  int mypid = getpid();
  
  // Wait for MEDIUM to start
  while(!medium_started) {
    // Busy wait
  }
  
  // Give MEDIUM time to potentially preempt LOW
  for(int i = 0; i < 20000000; i++);
  
  // Now set our priority
  setpriority(1);
  
  printf("\n[PID=%d][HIGH] Starting with priority 1\n", mypid);
  printf("[PID=%d][HIGH] Need to acquire the lock that LOW holds...\n", mypid);
  printf("[PID=%d][HIGH] *** PRIORITY INHERITANCE SHOULD TRIGGER NOW ***\n\n", mypid);
  
  high_started = 1;
  
  // Try to acquire the lock - this should boost LOW
  test_acquire();
  
  printf("\n[PID=%d][HIGH] Lock acquired!\n", mypid);
  printf("[PID=%d][HIGH] Doing quick work...\n", mypid);
  
  cpu_work(5000000);
  
  printf("[PID=%d][HIGH] Releasing lock...\n", mypid);
  test_release();
  
  printf("[PID=%d][HIGH] DONE\n\n", mypid);
}

int
main(void)
{
  print_header();
  
  printf("Starting processes in sequence...\n\n");
  
  // Reset flags
  low_has_lock = 0;
  medium_started = 0;
  high_started = 0;
  
  // Create LOW priority process
  int pid_low = fork();
  if(pid_low == 0) {
    low_priority_process();
    exit(0);
  }
  
  // Wait a bit for LOW to start and acquire lock
  for(int i = 0; i < 15000000; i++);
  
  // Create MEDIUM priority process
  int pid_med = fork();
  if(pid_med == 0) {
    medium_priority_process();
    exit(0);
  }
  
  // Wait a bit for MEDIUM to start
  for(int i = 0; i < 10000000; i++);
  
  // Create HIGH priority process
  int pid_high = fork();
  if(pid_high == 0) {
    high_priority_process();
    exit(0);
  }
  
  // Wait for all children to complete
  printf("[PARENT] Waiting for all processes to complete...\n\n");
  
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n========================================================\n");
  printf("                   TEST COMPLETE\n");
  printf("========================================================\n\n");
  printf("What to look for:\n");
  printf("  1. [PRIORITY INHERITANCE] message when HIGH waits\n");
  printf("  2. LOW's priority changes from 10 to 1\n");
  printf("  3. [PRIORITY RESTORE] when LOW releases lock\n");
  printf("  4. LOW finishes before or during MEDIUM\n\n");
  
  exit(0);
}
