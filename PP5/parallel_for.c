#include <pthread.h>
#include <stdio.h>

typedef struct {
    int start, end, inc, thread_id;
    void *(*functor)(int, void*);
    void *arg;
} ThreadData;

void *thread_function(void *arg) {
    ThreadData *data = (ThreadData*) arg;
    for (int i = data->start; i < data->end; i += data->inc) {
        data->functor(i, data->arg);
    }
    return NULL;
}

void parallel_for(int start, int end, int inc, void *(*functor)(int, void*), void *arg, int num_threads) {
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int chunk_size = (end - start) / num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = start + i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? end : thread_data[i].start + chunk_size;
        thread_data[i].inc = inc;
        thread_data[i].functor = functor;
        thread_data[i].arg = arg;
        pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}