#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

// Structure to pass data to threads
typedef struct {
    int thread_id;      // Thread identifier
    double x;           // Original x value (will be reduced in thread)
    int k;              // Term index (0, 1, 2, ...)
    double term;        // Calculated term value (output)
} ThreadData;

// Function to reduce angle to [-π, π] range
double reduce_angle(double angle) {
    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;
    
    // Reduce to [0, 2π] range using fmod
    angle = fmod(angle, TWO_PI);
    
    // Reduce to [-π, π] range
    if (angle > PI) {
        angle -= TWO_PI;
    } else if (angle < -PI) {
        angle += TWO_PI;
    }
    
    return angle;
}

// Thread function to calculate a single Taylor term
void* calculate_term(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    // Get thread's PID
    pid_t tid = getpid();
    
    // Reduce x to [-π, π] range before calculations
    double x_reduced = reduce_angle(data->x);
    
    int exponent = 2 * data->k + 1;            // 1, 3, 5, 7, ...
    int sign = (data->k % 2 == 0) ? 1 : -1;    // (-1)^k
    
    // Calculate x^(2k+1) with reduced x
    double x_power = 1.0;
    for (int j = 0; j < exponent; j++) {
        x_power *= x_reduced;
    }
    
    // Calculate (2k+1)! 
    double factorial = 1.0;
    for (int j = 1; j <= exponent; j++) {
        factorial *= j;
    }
    
    // Calculate term: (-1)^k * x^(2k+1) / (2k+1)!
    data->term = sign * x_power / factorial;
    
    printf("Thread %d (PID: %d): Term[%d] = %e\n", 
           data->thread_id, tid, data->k, data->term);
    
    return NULL;
}

int main() {
    int K, N, n;
    
    // Get user input
    printf("Enter K (number of y values): ");
    scanf("%d", &K);
    
    printf("Enter N (parameter for sin function): ");
    scanf("%d", &N);
    
    printf("Enter n (number of Taylor series terms): ");
    scanf("%d", &n);
    
    // Validate input
    if (K <= 0 || N <= 0 || n <= 0) {
        printf("Error: all values must be positive\n");
        return 1;
    }
    
    const double PI = 3.14159265358979323846;
    
    // Open output file
    FILE* output_file = fopen("sin_results.txt", "w");
    if (!output_file) {
        perror("Error opening file");
        return 1;
    }
    
    fprintf(output_file, "Results for sin(2*PI*i/N)\n");
    fprintf(output_file, "K=%d, N=%d, n=%d\n\n", K, N, n);
    fprintf(output_file, "i\tTaylor Approximation\tExact Value\t\tError\n");
    
    printf("\n=== Starting calculations ===\n");
    printf("Main Process (PID: %d)\n\n", getpid());
    
    // Calculate K values of the function
    for (int i = 0; i < K; i++) {
        double x = 2 * PI * i / N;
        
        // Reduce x for display purposes too
        double x_reduced = reduce_angle(x);
        
        printf("\n--- Calculating y[%d] = sin(2*PI*%d/%d) ---\n", i, i, N);
        printf("Original x = %lf, Reduced x = %lf\n", x, x_reduced);
        
        // Prepare thread data
        ThreadData* thread_data = malloc(n * sizeof(ThreadData));
        pthread_t* threads = malloc(n * sizeof(pthread_t));
        
        if (!thread_data || !threads) {
            printf("Memory allocation error\n");
            fclose(output_file);
            return 1;
        }
        
        // Create threads for each Taylor term
        for (int k = 0; k < n; k++) {
            thread_data[k].thread_id = k;
            thread_data[k].x = x;          // Original x (will be reduced in thread)
            thread_data[k].k = k;          // Term index
            thread_data[k].term = 0.0;     // Will be calculated by thread
            
            // Create thread
            if (pthread_create(&threads[k], NULL, calculate_term, &thread_data[k]) != 0) {
                perror("Thread creation error");
                free(thread_data);
                free(threads);
                fclose(output_file);
                return 1;
            }
        }
        
        // Wait for all threads to complete
        for (int k = 0; k < n; k++) {
            pthread_join(threads[k], NULL);
        }
        
        // Master process sums all terms
        double taylor_sum = 0.0;
        for (int k = 0; k < n; k++) {
            taylor_sum += thread_data[k].term;
        }
        
        // Calculate actual value for comparison
        double actual_value = sin(x);
        double error = fabs(taylor_sum - actual_value);
        
        // Display results
        printf("\nMain Process (PID: %d):\n", getpid());
        printf("  Taylor series sum: %lf\n", taylor_sum);
        printf("  Exact value:       %lf\n", actual_value);
        printf("  Error:             %e\n", error);
        
        // Write to file
        fprintf(output_file, "%d\t%lf\t\t%lf\t\t%e\n", 
                i, taylor_sum, actual_value, error);
        
        // Clean up
        free(thread_data);
        free(threads);
    }
    
    fclose(output_file);
    printf("\n=== Calculations completed ===\n");
    printf("Results saved to 'sin_results.txt'\n");
    
    return 0;
}
