#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdatomic.h>

typedef long long ll;

ll N = 8 * 1000000;
ll thread_cnt;

typedef struct {
    int* array;
    long long start;
    long long end;
    long long result;
} ThreadData;

void* sum_array(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long sum = 0;
    for (long long i = data->start; i < data->end; i++) {
        sum += data->array[i];
    }
    data->result = sum;
    return NULL;
}

int main(int argc, char* argv[]) {
    long thread;
    pthread_t* thread_handles;

    thread_cnt = strtol(argv[1], NULL, 10);
    thread_handles = malloc(thread_cnt * sizeof(pthread_t));

    int* array = malloc(N * sizeof(int));
    for (long long i = 0; i < N; i++) {
        array[i] = rand() % 100;
    }

    ThreadData threadData[thread_cnt];
    ll length_per_thread = N / thread_cnt;

    struct timeval start, end;
    gettimeofday(&start, NULL);  

    for (thread = 0; thread < thread_cnt; thread++) {
        threadData[thread].array = array;
        threadData[thread].start = thread * length_per_thread;
        threadData[thread].end = (thread + 1) * length_per_thread;
        pthread_create(&thread_handles[thread], NULL, sum_array, (void*)&threadData[thread]);
    }

    ll total_sum = 0;
    // 
    for (thread = 0; thread < thread_cnt; thread++) {
        pthread_join(thread_handles[thread], NULL);
        total_sum += threadData[thread].result; 
    }

    gettimeofday(&end, NULL);  
    double time_taken = end.tv_sec + end.tv_usec / 1e6 - start.tv_sec - start.tv_usec / 1e6;

    printf("Total sum: %lld\n", total_sum);
    printf("Cost time: %f seconds\n", time_taken);

    free(array);
    return EXIT_SUCCESS;
}