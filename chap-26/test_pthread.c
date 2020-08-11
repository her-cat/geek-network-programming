#include "../lib/common.h"
#include <pthread.h>

int another_shared = 0;

void *thread_run(void *arg) {
    int *calculator = (int *) arg;
    printf("tid: %ld \n", pthread_self());
    for (int i = 0; i < 1000; i++) {
        *calculator += 1;
        another_shared += 1;
    }
}

/*
 * pthread_exit(): 等待所有子线程终止，随后父线程也自己终止。
 * pthread_cancel(): 主动终止某个线程。
 * pthread_detach(): 分离某个线程，在子线程终止后能够自动回收相关的线程资源。
 */
int main(int argc, char *argv) {
    int calculator;
    pthread_t tid1, tid2;

    /* 创建线程 */
    pthread_create(&tid1, NULL, thread_run, &calculator);
    pthread_create(&tid2, NULL, thread_run, &calculator);

    /* 等待线程结束 */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("calculator: %d \n", calculator);
    printf("another_shared: %d \n", another_shared);

    return EXIT_SUCCESS;
}
