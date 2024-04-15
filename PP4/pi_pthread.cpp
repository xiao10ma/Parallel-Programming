#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

long thread_cnt;
int counter;
pthread_mutex_t mutex1;
pthread_cond_t cond_var;

double max_elapsed_time = 0.0; // 存储最大时间
pthread_mutex_t max_time_mutex = PTHREAD_MUTEX_INITIALIZER; // 保护最大时间的互斥锁

struct ThreadData {
    int thread_id;        
    long long num_points;       // 每个线程生成的点数
    long long count_in_circle;  // 落在圆内的点数
};

void* generate_points(void* arg) {
    struct timespec start_time, end_time;
    double elapsed_time;

    // Barrier
    pthread_mutex_lock(&mutex1);
    counter++;
    if (counter == thread_cnt) {
        counter = 0;
        pthread_cond_broadcast(&cond_var);
    }
    else {
        while (pthread_cond_wait(&cond_var, &mutex1) != 0);
    }
    pthread_mutex_unlock(&mutex1);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    ThreadData* data = (ThreadData*)arg;
    double x, y;
    double radius = 0.5; 

    srand(time(NULL) ^ (unsigned int)data->thread_id);

    for (int i = 0; i < data->num_points; i++) {
        x = (double)rand() / RAND_MAX; // 生成0到1之间的随机数
        y = (double)rand() / RAND_MAX;
        if (sqrt((x - 0.5) * (x - 0.5) + (y - 0.5) * (y - 0.5)) <= radius) {
            data->count_in_circle++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    elapsed_time = (end_time.tv_sec - start_time.tv_sec);
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

    // 更新全局最大时间
    pthread_mutex_lock(&max_time_mutex); // 锁定互斥锁以安全更新
    if (elapsed_time > max_elapsed_time) {
        max_elapsed_time = elapsed_time; // 更新最大时间
    }
    pthread_mutex_unlock(&max_time_mutex); // 解锁互斥锁
    return NULL;
}

int main(int argc, char* argv[]) {
    long long total_points = 65536; // 总点数

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex1, NULL);
    pthread_cond_init(&cond_var, NULL);

    thread_cnt = strtol(argv[1], NULL, 10);
    pthread_t threads[thread_cnt];
    ThreadData data[thread_cnt];

    long long points_per_thread = total_points / thread_cnt;

    for (int i = 0; i < thread_cnt; i++) {
        data[i].thread_id = i;  // 分配线程ID
        data[i].num_points = points_per_thread;
        pthread_create(&threads[i], NULL, generate_points, (void*)&data[i]);
    }

    long long total_in_circle = 0;
    for (int i = 0; i < thread_cnt; i++) {
        pthread_join(threads[i], NULL);
        total_in_circle += data[i].count_in_circle;
    }

    double pi_estimate = 4.0 * total_in_circle / total_points;
    
    printf("Cost time: %f\n", max_elapsed_time);
    printf("Estimated Pi: %.6f\n", pi_estimate);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex1);
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&max_time_mutex);

    return 0;
}
