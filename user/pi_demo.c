// user/pi_demo.c
// Clear and visual demonstration of priority inheritance

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int stage = 0;
volatile int test_complete = 0;

void
print_banner(char *title)
{
  printf("\n");
  printf("═══════════════════════════════════════════════════════════\n");
  printf("  %s\n", title);
  printf("═══════════════════════════════════════════════════════════\n");
}

void
print_step(int step, char *description)
{
  printf("\n┌─ STEP %d ────────────────────────────────────────────────┐\n", step);
  printf("│ %s\n", description);
  printf("└──────────────────────────────────────────────────────────┘\n");
}

void
low_task(void)
{
  setpriority(10);
  
  print_step(1, "LOW PRIORITY (10) - Acquiring Lock");
  printf("   [LOW] I'm acquiring the kernel lock...\n");
  
  test_acquire();
  
  printf("   [LOW] ✓ Lock acquired!\n");
  printf("   [LOW] Starting LONG critical section work...\n");
  printf("   [LOW] My current priority: %d\n", getpriority());
  
  stage = 1;
  
  // Do work in chunks so we can see priority changes
  for(int chunk = 0; chunk < 4; chunk++) {
    printf("\n   [LOW] Working... chunk %d/4 (priority=%d)\n", 
           chunk + 1, getpriority());
    cpu_work(25000000);
  }
  
  printf("\n   [LOW] Critical section complete!\n");
  printf("   [LOW] Releasing lock...\n");
  printf("   [LOW] My final priority: %d\n", getpriority());
  
  test_release();
  
  printf("   [LOW] ✓ Lock released! I'm done.\n");
}

void
medium_task(void)
{
  // Wait for LOW to acquire lock
  while(stage < 1)
    ;
  
  // Small delay
  for(int i = 0; i < 5000000; i++)
    ;
  
  setpriority(5);
  
  print_step(2, "MEDIUM PRIORITY (5) - CPU Intensive Work");
  printf("   [MEDIUM] I don't need the lock, just doing CPU work\n");
  printf("   [MEDIUM] My priority: %d (higher than LOW's 10)\n", getpriority());
  printf("   [MEDIUM] Normally I would preempt LOW...\n");
  
  stage = 2;
  
  printf("\n   [MEDIUM] Starting my work...\n");
  cpu_work(30000000);
  
  printf("\n   [MEDIUM] ✓ My work is done!\n");
}

void
high_task(void)
{
  // Wait for MEDIUM to start
  while(stage < 2)
    ;
  
  // Give MEDIUM a moment to start working
  for(int i = 0; i < 10000000; i++)
    ;
  
  setpriority(1);
  
  print_step(3, "HIGH PRIORITY (1) - Needs the Lock!");
  printf("   [HIGH] I need the lock that LOW has!\n");
  printf("   [HIGH] My priority: %d (HIGHEST)\n", getpriority());
  printf("   [HIGH] LOW's priority: 10 (very low)\n");
  printf("   [HIGH] Attempting to acquire lock...\n");
  
  printf("\n   ╔══════════════════════════════════════════════════════╗\n");
  printf("   ║  🔔 PRIORITY INHERITANCE SHOULD HAPPEN NOW!         ║\n");
  printf("   ║  → LOW should be boosted from priority 10 to 1      ║\n");
  printf("   ║  → Watch the kernel messages below!                 ║\n");
  printf("   ╚══════════════════════════════════════════════════════╝\n\n");
  
  int start_time = uptime();
  
  test_acquire();
  
  int wait_time = uptime() - start_time;
  
  printf("\n   [HIGH] ✓ Got the lock!\n");
  printf("   [HIGH] I waited %d ticks for the lock\n", wait_time);
  printf("   [HIGH] Doing my quick work...\n");
  
  cpu_work(5000000);
  
  printf("\n   [HIGH] Releasing lock...\n");
  test_release();
  
  printf("   [HIGH] ✓ All done!\n");
  
  test_complete = 1;
}

void
print_explanation(void)
{
  print_banner("WHAT IS PRIORITY INHERITANCE?");
  
  printf("\n📚 WITHOUT Priority Inheritance:\n");
  printf("   1. LOW (priority 10) holds a lock\n");
  printf("   2. MEDIUM (priority 5) runs and preempts LOW\n");
  printf("   3. HIGH (priority 1) waits for the lock\n");
  printf("   4. ❌ HIGH is blocked by MEDIUM (priority inversion!)\n");
  printf("   5. HIGH waits a LONG time\n");
  
  printf("\n✅ WITH Priority Inheritance:\n");
  printf("   1. LOW (priority 10) holds a lock\n");
  printf("   2. MEDIUM (priority 5) runs and preempts LOW\n");
  printf("   3. HIGH (priority 1) tries to get the lock\n");
  printf("   4. ⚡ LOW is BOOSTED to priority 1 (inherits from HIGH)\n");
  printf("   5. LOW preempts MEDIUM and finishes quickly\n");
  printf("   6. HIGH gets the lock immediately\n");
  printf("   7. MEDIUM finishes last\n");
  
  printf("\n🎯 Benefits:\n");
  printf("   • No unbounded priority inversion\n");
  printf("   • High-priority tasks aren't delayed by low-priority ones\n");
  printf("   • Predictable real-time behavior\n");
  
  printf("\n");
}

void
print_results(void)
{
  print_banner("TEST RESULTS");
  
  printf("\n✅ Priority Inheritance is WORKING if you saw:\n");
  printf("   1. Kernel message: \"Priority Inheritance: ... boosted to 1\"\n");
  printf("   2. LOW finished quickly after HIGH started waiting\n");
  printf("   3. LOW's priority changed from 10 → 1 → 10\n");
  
  printf("\n❌ Priority Inheritance is NOT working if:\n");
  printf("   1. No \"Priority Inheritance\" kernel message\n");
  printf("   2. MEDIUM finished before LOW\n");
  printf("   3. HIGH waited a very long time\n");
  
  printf("\n");
}

int
main(void)
{
  printf("\n\n");
  printf("╔═════════════════════════════════════════════════════════════╗\n");
  printf("║                                                             ║\n");
  printf("║       PRIORITY INHERITANCE DEMONSTRATION                    ║\n");
  printf("║       Using Real Kernel Locks in xv6                        ║\n");
  printf("║                                                             ║\n");
  printf("╚═════════════════════════════════════════════════════════════╝\n");
  
  print_explanation();
  
  printf("\n⏳ Starting test in 2 seconds...\n");
  for(int i = 0; i < 20000000; i++)
    ;
  
  print_banner("TEST EXECUTION");
  printf("\nWatch carefully for kernel messages about priority changes!\n");
  
  stage = 0;
  test_complete = 0;
  
  // Create the three processes
  if(fork() == 0) {
    low_task();
    exit(0);
  }
  
  if(fork() == 0) {
    medium_task();
    exit(0);
  }
  
  if(fork() == 0) {
    high_task();
    exit(0);
  }
  
  // Wait for all children to complete
  for(int i = 0; i < 3; i++) {
    wait(0);
  }
  
  print_results();
  
  printf("\n✨ Test completed successfully!\n\n");
  
  exit(0);
}
