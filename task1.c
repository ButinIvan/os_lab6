#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

void print_process_info(const char* process_name) {
    // Get current time with milliseconds
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    struct tm* tm_info = localtime(&tv.tv_sec);
    
    // Print process information
    printf("%s: PID = %d, PPID = %d, Time = %02d:%02d:%02d:%03ld\n",
           process_name,
           getpid(),      // Current process ID
           getppid(),     // Parent process ID
           tm_info->tm_hour,
           tm_info->tm_min,
           tm_info->tm_sec,
           tv.tv_usec / 1000);  // Convert microseconds to milliseconds
}

int main() {
    printf("=== Program Start ===\n");
    
    // Print parent process info FIRST
    print_process_info("Parent Process");
    
    // First fork() call
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        // Error creating process
        perror("Error in first fork() call");
        exit(1);
    }
    
    if (pid1 == 0) {
        // First child process
        print_process_info("Child Process 1");
        
        // Exit child process
        exit(0);
    }
    
    // Parent process continues execution
    // Second fork() call
    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        // Error creating process
        perror("Error in second fork() call");
        exit(1);
    }
    
    if (pid2 == 0) {
        // Second child process
        print_process_info("Child Process 2");
        
        // Exit child process
        exit(0);
    }
    
    // Parent process waits for child processes to complete
    // Small delay to ensure children have time to execute
    usleep(100000);  // 100ms delay
    
    // Execute ps -x command in parent process
    printf("\n=== Executing command: ps -x | grep %d ===\n", getpid());
    
    char command[100];
    // Create command to show our processes
    sprintf(command, "ps -x | grep %d", getpid());
    
    // Execute system command
    int result = system(command);
    if (result != 0) {
        printf("Command execution returned non-zero code: %d\n", result);
    }
    
    // Wait for both child processes to finish
    // This prevents zombie processes
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    
    printf("=== Program End ===\n");
    
    return 0;
}
