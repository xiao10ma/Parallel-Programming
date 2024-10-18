#include <iostream>
#include <pthread.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

int a, b, c;
double A_val, B_val, C_val, D_val;
int computation_stage;
pthread_mutex_t stage_mutex;
int has_solution;

// A = b^2
void* compute_A(void* rank) {
    A_val = b * b;
    pthread_mutex_lock(&stage_mutex);
    computation_stage++;
    pthread_mutex_unlock(&stage_mutex);
    return nullptr;
}

// B = (-4) * a * c
void* compute_B(void* rank) {
    B_val = (-4) * a * c;
    pthread_mutex_lock(&stage_mutex);
    computation_stage++;
    pthread_mutex_unlock(&stage_mutex);
    return nullptr;
}

// C = sqrt(A + B)
void* compute_C(void* rank) {
    while (computation_stage < 2) {
        // Wait for A and B to be calculated
    }
    C_val = A_val + B_val;
    if (C_val >= 0) {
        has_solution = 1;
        C_val = sqrt(C_val);
    }
    return nullptr;
}

// D = 2 * a
void* compute_D(void* rank) {
    D_val = 2 * a;
    return nullptr;
}

void solve_sequential() {
    cout << "Solving sequentially...\n";
    double A_temp = b * b;
    double B_temp = (-4) * a * c;
    double C_temp = A_temp + B_temp;
    double D_temp = 2 * a;
    
    if (C_temp >= 0) {
        C_temp = sqrt(C_temp);
        double x1 = (-b + C_temp) / D_temp;
        double x2 = (-b - C_temp) / D_temp;
        printf("x1 = %lf, x2 = %lf\n", x1, x2);
    } else {
        cout << "No real solution.\n";
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    a = rand() % 201 - 100;
    b = rand() % 201 - 100;
    c = rand() % 201 - 100;
    
    cout << "Equation: (" << a << "x^2) + (" << b << "x) + (" << c << ") = 0\n";
    
    A_val = B_val = C_val = D_val = 0;
    computation_stage = 0;
    has_solution = 0;
    
    timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    
    pthread_mutex_init(&stage_mutex, nullptr);
    
    pthread_t threads[4];
    pthread_create(&threads[0], nullptr, compute_A, nullptr);
    pthread_create(&threads[1], nullptr, compute_B, nullptr);
    pthread_create(&threads[2], nullptr, compute_C, nullptr);
    pthread_create(&threads[3], nullptr, compute_D, nullptr);
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], nullptr);
    }

    if (has_solution) {
        double x1 = (-b + C_val) / D_val;
        double x2 = (-b - C_val) / D_val;
        printf("x1 = %lf, x2 = %lf\n", x1, x2);
    } else {
        cout << "No real solution.\n";
    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1e6 + (end_time.tv_nsec - start_time.tv_nsec) / 1e3;
    cout << "Parallel solution time: " << elapsed_time / 1e6 << "ms\n";
    
    timespec seq_start, seq_end;
    clock_gettime(CLOCK_REALTIME, &seq_start);
    solve_sequential();
    clock_gettime(CLOCK_REALTIME, &seq_end);
    double seq_elapsed_time = (seq_end.tv_sec - seq_start.tv_sec) * 1e6 + (seq_end.tv_nsec - seq_start.tv_nsec) / 1e3;
    cout << "Sequential solution time: " << seq_elapsed_time / 1e6 << "ms\n";
    
    return 0;
}
