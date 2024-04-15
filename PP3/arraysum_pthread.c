#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

long thread_cnt;
float* A;
const long N = 128 * 1000000;
long avg_len;
float sum = 0.;

pthread_mutex_t mutex;

void initialize_array(float* array, int length) {
    // 设置随机种子，通常使用当前时间作为种子
    srand((unsigned)time(NULL));

    for (int i = 0; i < length; i++) {
        array[i] = (float)rand() / (float)(RAND_MAX) * 10.0;
    }
}

void *array_sum(void* rank) {
    long my_rank = (long) rank;
    long start_ind = my_rank * avg_len;
    long end_ind = (my_rank + 1) * avg_len;

    float local_sum = 0;
    for (int i = start_ind; i < end_ind; i ++) {
        local_sum += A[i];
    }

    pthread_mutex_lock(&mutex);
    sum += local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL); // 计时开始
    long thread;
    pthread_t* thread_handles;
    pthread_mutex_init(&mutex, NULL);

    thread_cnt = strtol(argv[1], NULL, N);
    thread_handles = malloc(thread_cnt * sizeof(pthread_t));
    avg_len = N / thread_cnt;

    A = (float *)malloc(sizeof(float) * N);
    initialize_array(A, 10);
    
    for (thread = 0; thread < thread_cnt; thread ++) {
        pthread_create(&thread_handles[thread], NULL, array_sum, (void *) thread);
    }
    
    for (thread = 0; thread < thread_cnt; thread ++) {
        pthread_join(thread_handles[thread], NULL);
    }

    gettimeofday(&end, NULL); // 计时结束
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Cost time: %f\n", time_spent);
    
    free(thread_handles);
    free(A);
    pthread_mutex_destroy(&mutex);

    return 0;
}