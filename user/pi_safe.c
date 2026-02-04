#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int stage = 0;

int
main(void)
{
  printf("\n=== SAFE PRIORITY INHERITANCE TEST ===\n\n");
  
  stage = 0;
  
  // LOW priority
  if(fork() == 0) {
    setpriority(10);
    int pid = getpid();
    printf("[%d][LOW-10] Acquiring lock\n", pid);
    test_acquire();
    stage = 1;
    printf("[%d][LOW-10] Got lock, working (pri=%d)\n", pid, getpriority());
    cpu_work(100000000);
    printf("[%d][LOW-10] Done working, releasing (pri=%d)\n", pid, getpriority());
    test_release();
    printf("[%d][LOW-10] Released (pri=%d)\n", pid, getpriority());
    exit(0);
  }
  
  // Wait for LOW to get lock
  while(stage < 1);
  for(int i = 0; i < 10000000; i++);
  
  // MEDIUM priority
  if(fork() == 0) {
    setpriority(5);
    int pid = getpid();
    printf("[%d][MED-5] Working (no lock)\n", pid);
    stage = 2;
    cpu_work(60000000);
    printf("[%d][MED-5] Done\n", pid);
    exit(0);
  }
  
  // Wait for MEDIUM to start
  while(stage < 2);
  for(int i = 0; i < 10000000; i++);
  
  // HIGH priority
  if(fork() == 0) {
    setpriority(1);
    int pid = getpid();
    printf("\n[%d][HIGH-1] Acquiring lock (should boost LOW)\n", pid);
    test_acquire();
    printf("[%d][HIGH-1] Got lock\n", pid);
    test_release();
    printf("[%d][HIGH-1] Done\n", pid);
    exit(0);
  }
  
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n=== TEST COMPLETE ===\n");
  printf("Look for [INHERITANCE] and [PI-RESTORE] messages\n\n");
  
  exit(0);
}
