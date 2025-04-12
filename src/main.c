#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* countNumbers(void* arg) {

    int* limit = (int*)arg;
    for (int i=1; i<=*limit; i++) {
        printf("Thread 1: Counting %d\n", i);
        usleep(500000); // 0.5 seconds
    }
    
    printf("Thread 1: Counting complete\n");
    return NULL;
}

void* printLetters(void* arg) {

    int count = *(int*)arg;
    for (int i=0; i<count; i++) {
        printf("Thread 2: Printing %c\n", 'A' + i);
        usleep(700000); // 0.7 seconds
    }
    
    printf("Thread 2: Printing complete\n");
    return NULL;
}

int main() {

    printf("THREAD EXAMPLE\n");
    pthread_t thread1, thread2;
    int c1 = 5;
    int c2 = 5;
    
    pthread_create(&thread1, NULL, countNumbers, &c1);
    pthread_create(&thread2, NULL, printLetters, &c2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("All threads completed successfully!\n");
    return 0;
}