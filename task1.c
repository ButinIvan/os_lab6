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
	pid_t pid1, pid2;

	printf("Запуск программы\n");
	
	pid1 = fork();

	if (pid1 < 0) {
		perror("Ошибка первого fork");
		return 1;
	}
  
	if (pid1 == 0) {
		print_process_info("Дочерний процесс 1");
		exit(0);
	} 
    else {
		pid2 = fork();
		
  	if (pid2 < 0) {
  	  perror("Ошибка второго fork");
  		return 1;
  	}
  
  	if (pid2 == 0) {
  		print_process_info("Дочерний процесс 2");
  		exit(0);
  	} 
    else {
  		print_process_info("Родительский процесс");
  		usleep(10000);
  
  		printf("\nВыполнение команды ps -x в родительском процессе\n");
  			
  		system("ps -x");
      
  		wait(NULL);
  		wait(NULL);
  	}
	}

	printf("Родительский процесс завершает работу\n");
	return 0;
}
