#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// 线程参数结构体
typedef struct {
    int thread_id;
    long points_inside;
    long points_total;
} ThreadData;

#define NUM_THREADS 4
#define NUM_POINTS 10000000 // 总的随机点数
long points_inside_total = 0;
pthread_mutex_t lock;

// 生成 [0,1] 范围的随机浮点数
double rand_double() {
    return rand() / (double)RAND_MAX;
}

// 线程函数，计算每个线程的积分区域内点数
void* monte_carlo_thread(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    long points_inside = 0;

    // 每个线程负责的点数
    long points_per_thread = data->points_total;

    // 生成随机点并检查是否在曲线下方
    for (long i = 0; i < points_per_thread; i++) {
        double x = rand_double();
        double y = rand_double();

        if (y <= x * x) {
            points_inside++;
        }
    }

    // 使用互斥锁更新全局计数
    pthread_mutex_lock(&lock);
    points_inside_total += points_inside;
    pthread_mutex_unlock(&lock);

    data->points_inside = points_inside;
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    long points_per_thread = NUM_POINTS / NUM_THREADS;
    
    srand(time(NULL));  // 随机数种子
    pthread_mutex_init(&lock, NULL);

    // 创建线程
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].points_inside = 0;
        thread_data[i].points_total = points_per_thread;
        pthread_create(&threads[i], NULL, monte_carlo_thread, &thread_data[i]);
    }

    // 等待线程结束
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 估算面积
    double area = (double)points_inside_total / NUM_POINTS;

    printf("估算的面积为: %lf\n", area);

    pthread_mutex_destroy(&lock);
    return 0;
}
