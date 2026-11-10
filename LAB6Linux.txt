#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>

typedef struct {
    sem_t sem;
    int counter;
} SharedData;

void process_logic(SharedData *shared, const char *process_name) {
    srand(time(NULL) ^ getpid());
    while (1) {
        sem_wait(&shared->sem);
        if (shared->counter >= 1000) {
            sem_post(&shared->sem);
            break;
        }
        int coin = (rand() % 2) + 1;
        while (coin == 2 && shared->counter < 1000) {
            shared->counter++;
            printf("[%s] a incrementat contorul la: %d\n", process_name, shared->counter);
            coin = (rand() % 2) + 1;
        }
        sem_post(&shared->sem);
        usleep(1000);
    }
}

int main() {
    SharedData *shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) exit(1);
    shared->counter = 0;
    if (sem_init(&shared->sem, 1, 1) != 0) exit(1);
    
    pid_t pid = fork();
    if (pid < 0) exit(1);
    else if (pid == 0) {
        process_logic(shared, "Proces COPIL");
        exit(0);
    } else {
        process_logic(shared, "Proces PARINTE");
        wait(NULL);
        printf("\nContor final: %d\n", shared->counter);
        sem_destroy(&shared->sem);
        munmap(shared, sizeof(SharedData));
    }
    return 0;
}
