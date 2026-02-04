// user/pi_detailed.c
// Detailed priority inheritance demonstration with clear logging
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void work(long n) {
  volatile long s = 0;
  for(long i = 0; i < n; i++) {
    s += i;
    // Yield periodically to let scheduler make decisions
    if(i % 500000 == 0) {
      cpu_work(1);  // Minimal syscall that includes yield
    }
  }
}

int main(void)
{
  printf("\n");
  printf("============================================================\n");
  printf("         PRIORITY INHERITANCE DEMONSTRATION\n");
  printf("============================================================\n");
  printf("  LOW    = priority 10\n");
  printf("  MEDIUM = priority 5\n");
  printf("  HIGH   = priority 1\n");
  printf("============================================================\n\n");

  // ---------- LOW PRIORITY PROCESS ----------
  int pid_low = fork();
  if(pid_low == 0) {
    setpriority(10);
    int pid = getpid();
    
    printf("[USER] PID=%d STARTED (priority=10, LOW)\n", pid);
    work(5000000);
    
    test_acquire();
    printf("[USER] PID=%d HOLDING lock, working...\n", pid);
    
    // Do work while holding lock
    for(int i = 0; i < 4; i++) {
      printf("[USER] PID=%d RUNNING (priority=%d)\n", pid, getpriority());
      work(15000000);
    }
    
    test_release();
    printf("[USER] PID=%d FINISHED (priority=%d)\n\n", pid, getpriority());
    exit(0);
  }

  // Let LOW acquire the lock first
  work(15000000);

  // ---------- HIGH PRIORITY PROCESS ----------
  int pid_high = fork();
  if(pid_high == 0) {
    setpriority(1);
    int pid = getpid();
    
    printf("[USER] PID=%d STARTED (priority=1, HIGH)\n", pid);
    work(5000000);
    
    printf("[USER] PID=%d TRYING to acquire lock...\n", pid);
    test_acquire();
    
    printf("[USER] PID=%d RUNNING with lock\n", pid);
    work(10000000);
    
    test_release();
    printf("[USER] PID=%d FINISHED\n\n", pid);
    exit(0);
  }

  // Let HIGH try to acquire before starting MEDIUM
  work(15000000);

  // ---------- MEDIUM PRIORITY PROCESS ----------
  int pid_med = fork();
  if(pid_med == 0) {
    setpriority(5);
    int pid = getpid();
    
    printf("[USER] PID=%d STARTED (priority=5, MEDIUM)\n", pid);
    printf("[USER] PID=%d Does NOT need the lock\n", pid);
    work(5000000);
    
    // Try to do work - should NOT run much if priority inheritance is working
    printf("[USER] PID=%d TRYING to run...\n", pid);
    work(100000000);  // Much more work - should be blocked
    
    printf("[USER] PID=%d FINISHED\n\n", pid);
    exit(0);
  }

  // Wait for all children
  wait(0);
  wait(0);
  wait(0);

  printf("============================================================\n");
  printf("                    ANALYSIS\n");
  printf("============================================================\n");
  printf("Expected behavior WITH priority inheritance:\n");
  printf("  1. LOW (pri=10) acquires lock\n");
  printf("  2. HIGH (pri=1) tries to acquire, BLOCKS\n");
  printf("  3. LOW gets BOOSTED to pri=1\n");
  printf("  4. MEDIUM (pri=5) cannot run (LOW has higher priority)\n");
  printf("  5. LOW finishes, priority RESTORED to 10\n");
  printf("  6. HIGH acquires lock and finishes\n");
  printf("  7. MEDIUM finally runs\n");
  printf("\n");
  printf("WITHOUT priority inheritance:\n");
  printf("  MEDIUM would run before LOW finishes (priority inversion!)\n");
  printf("============================================================\n\n");
  
  exit(0);
}
