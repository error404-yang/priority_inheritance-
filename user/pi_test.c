#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int stage = 0;

void
low_task(void)
{
  setpriority(10);
  printf("\n========================================\n");
  printf("[LOW-10] Starting\n");
  printf("========================================\n");
  
  test_acquire();  // Acquire kernel lock
  stage = 1;
  printf("\n[LOW-10] Got lock! Starting long work...\n");
  
  cpu_work(100000000);  // Long work while holding lock
  
  printf("\n[LOW-10] Work complete! Releasing lock...\n");
  test_release();
  
  printf("[LOW-10] Done!\n");
}

void
medium_task(void)
{
  while(stage < 1);  // Wait for LOW to get lock
  
  // Small delay
  for(int i = 0; i < 1000000; i++);
  
  setpriority(5);
  printf("\n========================================\n");
  printf("[MEDIUM-5] Starting CPU work (no lock needed)\n");
  printf("========================================\n");
  stage = 2;
  
  cpu_work(50000000);
  
  printf("\n[MEDIUM-5] Done!\n");
}

void
high_task(void)
{
  while(stage < 2);  // Wait for MEDIUM to start
  
  // Give MEDIUM time to potentially preempt LOW
  for(int i = 0; i < 10000000; i++);
  
  setpriority(1);
  printf("\n========================================\n");
  printf("[HIGH-1] Starting!\n");
  printf("[HIGH-1] Going to try acquiring lock...\n");
  printf("[HIGH-1] *** WATCH: LOW should be BOOSTED to priority 1! ***\n");
  printf("========================================\n");
  
  test_acquire();  // This should boost LOW to priority 1!
  
  printf("\n[HIGH-1] Got lock! (LOW should have been boosted)\n");
  printf("[HIGH-1] Doing quick work...\n");
  
  cpu_work(1000000);
  
  test_release();
  printf("\n[HIGH-1] Done!\n");
}

int
main(void)
{
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║   PRIORITY INHERITANCE DEMONSTRATION (KERNEL)      ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\nThis test uses REAL kernel locks with priority inheritance!\n");
  printf("\nExpected sequence:\n");
  printf("  1. LOW (pri=10) acquires kernel lock\n");
  printf("  2. MEDIUM (pri=5) starts CPU work (no lock)\n");
  printf("  3. HIGH (pri=1) tries to acquire same lock\n");
  printf("  4. **LOW boosted to priority 1**\n");
  printf("  5. LOW finishes quickly (now has priority 1)\n");
  printf("  6. HIGH gets lock immediately\n");
  printf("  7. MEDIUM continues/finishes\n");
  printf("\nStarting test in 3 seconds...\n");
  
  for(int i = 0; i < 3000000; i++);
  
  printf("\n>>> TEST STARTING <<<\n");
  
  stage = 0;
  
  if(fork() == 0) { low_task(); exit(0); }
  if(fork() == 0) { medium_task(); exit(0); }
  if(fork() == 0) { high_task(); exit(0); }
  
  // Wait for all children
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║              TEST COMPLETED                        ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\nLook at the kernel messages above!\n");
  printf("You should see LOW being boosted from priority 10 to 1\n");
  printf("when HIGH tried to acquire the lock.\n\n");
  
  exit(0);
}
