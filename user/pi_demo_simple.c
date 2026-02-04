#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int stage = 0;

void
low_task(void)
{
  setpriority(10);
  
  printf("\n[STEP 1] LOW (priority 10) starting\n");
  printf("[LOW] Acquiring lock...\n");
  
  test_acquire();
  
  printf("[LOW] Lock acquired! Starting work...\n");
  stage = 1;
  
  // Do work - check priority during work
  for(int chunk = 0; chunk < 4; chunk++) {
    cpu_work(25000000);
    printf("[LOW] Work chunk %d/4 done (priority=%d)\n", chunk+1, getpriority());
  }
  
  printf("[LOW] Work complete! Releasing lock...\n");
  test_release();
  printf("[LOW] Done!\n");
}

void
medium_task(void)
{
  // Wait for LOW to start
  while(stage < 1)
    ;
  
  // Small delay
  for(int i = 0; i < 5000000; i++)
    ;
  
  setpriority(5);
  printf("\n[STEP 2] MEDIUM (priority 5) starting CPU work\n");
  stage = 2;
  
  cpu_work(30000000);
  printf("[MEDIUM] Work done!\n");
}

void
high_task(void)
{
  // Wait for MEDIUM to start
  while(stage < 2)
    ;
  
  // Give MEDIUM time to start
  for(int i = 0; i < 10000000; i++)
    ;
  
  setpriority(1);
  printf("\n[STEP 3] HIGH (priority 1) starting\n");
  printf("[HIGH] Trying to acquire lock...\n");
  printf("[HIGH] *** WATCH: Priority inheritance should happen! ***\n");
  
  test_acquire();
  
  printf("[HIGH] Got lock! Doing quick work...\n");
  cpu_work(5000000);
  
  test_release();
  printf("[HIGH] Done!\n");
}

int
main(void)
{
  printf("\n========================================\n");
  printf("  PRIORITY INHERITANCE TEST\n");
  printf("========================================\n\n");
  
  printf("Watch for kernel messages showing:\n");
  printf("  - Priority boost (10 -> 1)\n");
  printf("  - Priority restore (1 -> 10)\n\n");
  
  if(fork() == 0) { low_task(); exit(0); }
  if(fork() == 0) { medium_task(); exit(0); }
  if(fork() == 0) { high_task(); exit(0); }
  
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n========================================\n");
  printf("  TEST COMPLETE\n");
  printf("========================================\n\n");
  
  exit(0);
}
