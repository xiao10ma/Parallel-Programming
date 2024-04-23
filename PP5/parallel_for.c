#include <pthread.h>
#include <stdlib.h>

struct for_index {
	int start; 
	int end;
	int increment;
};

pthread_t* thread_handles;

void parallel_for (int start, int end, int increment, void*(*functor)(void*), void *arg, int num_threads) {
    int cnt = end - start;
    int threads = cnt >= num_threads ? num_threads : cnt;
    thread_handles = (pthread_t*)malloc(threads * sizeof(pthread_t));
    int average_loop = cnt / threads;
    for (int thread = 0; thread < threads; thread ++) {
        struct for_index* idx = (struct for_index*)malloc(sizeof(struct for_index));
        idx->start = average_loop * thread;
        idx->end = average_loop * (thread + 1);
        idx->increment = increment;
        pthread_create(&thread_handles[thread], NULL, functor, (void*) idx);
    }
    for (int thread = 0; thread < threads; thread ++) {
        pthread_join(thread_handles[thread], NULL);
    }
    //free(thread_handles);
}