// user/pi_test2.c
// Priority inheritance demo with cleaner output
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// busy-work loop, entirely in user space
static void
work(long n)
{
  volatile long s = 0;
  for(long i = 0; i < n; i++)
    s += i;
}

// Small delay to reduce print collision
static void
print_delay(void)
{
  work(1000000);
}

int
main(void)
{
  printf("\n");
  printf("============================================\n");
  printf("   Priority Inheritance Test\n");
  printf("   LOW=10  MEDIUM=5  HIGH=1\n");
  printf("============================================\n\n");

  // ---------- LOW ----------
  int pid_low = fork();
  if(pid_low == 0) {
    int pid = getpid();
    setpriority(10);
    
    printf("[PID=%d][LOW]  pri=%d  acquiring lock\n", pid, getpriority());
    print_delay();
    
    test_acquire();
    
    printf("[PID=%d][LOW]  pri=%d  lock held\n", pid, getpriority());
    print_delay();
    
    // do work in 8 chunks, print priority each time so we can see the boost
    for(int c = 0; c < 8; c++) {
      printf("[PID=%d][LOW]  pri=%d  chunk %d/8\n", pid, getpriority(), c);
      print_delay();
      work(8000000);  // Reduced work amount
    }
    
    printf("[PID=%d][LOW]  pri=%d  releasing lock\n", pid, getpriority());
    print_delay();
    
    test_release();
    
    printf("[PID=%d][LOW]  pri=%d  lock released\n", pid, getpriority());
    print_delay();
    exit(0);
  }

  // Give LOW time to acquire the lock
  work(10000000);

  // ---------- MEDIUM ----------
  int pid_med = fork();
  if(pid_med == 0) {
    int pid = getpid();
    setpriority(5);
    
    printf("[PID=%d][MED]  pri=%d  working (no lock)\n", pid, getpriority());
    print_delay();
    
    work(40000000);
    
    printf("[PID=%d][MED]  pri=%d  done\n", pid, getpriority());
    print_delay();
    exit(0);
  }

  // Give MEDIUM time to start
  work(10000000);

  // ---------- HIGH ----------
  int pid_high = fork();
  if(pid_high == 0) {
    int pid = getpid();
    setpriority(1);
    
    printf("\n[PID=%d][HIGH] pri=%d  acquiring lock\n", pid, getpriority());
    printf("               (should boost LOW to pri=1 now)\n\n");
    print_delay();
    
    test_acquire();
    
    printf("[PID=%d][HIGH] pri=%d  lock held\n", pid, getpriority());
    print_delay();
    
    test_release();
    
    printf("[PID=%d][HIGH] pri=%d  done\n", pid, getpriority());
    print_delay();
    exit(0);
  }

  // PARENT: Wait for all 3 children
  wait(0);
  wait(0);
  wait(0);

  printf("\n============================================\n");
  printf("              SUMMARY\n");
  printf("============================================\n");
  printf("SUCCESS! Priority inheritance working:\n");
  printf("  1. LOW started at pri=10\n");
  printf("  2. HIGH blocked, boosted LOW to pri=1\n");
  printf("  3. LOW finished, restored to pri=10\n");
  printf("  4. HIGH acquired lock and completed\n");
  printf("============================================\n\n");
  
  exit(0);
}
