#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include "codec.h"
#include "queue.c"
#include <time.h>


#define CHUNK_SIZE 1024

struct thread_args {
    int key;
    Queue* queue;
    char* output_list[CHUNK_SIZE];
};

//The function that each thread will execute to encrypt the data
void* thread_encrypt(void* threadargs) {
    struct thread_args* args = (struct thread_args*) threadargs;
    int key = args->key;
    Queue* queue = args->queue;
    char** output_list = args->output_list;
    while (!isEmpty(queue)) {
        Message message = dequeue(queue);
        char* encrypted_data = message.message;
        encrypt(encrypted_data, key);
        memcpy(output_list[message.index], encrypted_data, CHUNK_SIZE);
    }
    pthread_exit(NULL);
}

//The function that each thread will execute to decrypt the data
void* thread_decrypt(void* threadargs) {
    struct thread_args* args = (struct thread_args*) threadargs;
    int key = args->key;
    Queue* queue = args->queue;
    char** output_list = args->output_list;
    while (!isEmpty(queue)) {
        Message message = dequeue(queue);
        char* decrypted_data = message.message;
        decrypt(decrypted_data, key);
        memcpy(output_list[message.index], decrypted_data, CHUNK_SIZE);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // check arguments
    if (argc != 3) {
        printf("usage: key < file \n");
        printf("Data should arrive in the following way: ./coder <key> <command -e or -d>\n");
        return 0;
    }

    Queue* queue = createQueue();

    // get key and operation
    int key = atoi(argv[1]);
    char* operation = argv[2];

    // get number of available cores
    int num_of_cores = get_nprocs_conf();

    // create thread array
    pthread_t threads[num_of_cores - 1];

    // read input data and fill the queue
    char c;
    int counter = 0;
    int num_of_breaks = 0;
    char data[CHUNK_SIZE];
    while ((c = getchar()) != EOF) {
        data[counter] = c;
        counter++;
        if (counter == CHUNK_SIZE) {
            enqueue(queue, data, num_of_breaks);
            counter = 0;
            num_of_breaks++;
            memset(data, 0, CHUNK_SIZE);
        }
    }
    // enqueue the last chunk
    memset(data + counter, '\0', sizeof(data) - counter);
    enqueue(queue, data, num_of_breaks);

    // set up the output list
    char output_list[num_of_breaks + 1][CHUNK_SIZE];

    // set up thread arguments
    struct thread_args thread_args;
    thread_args.key = key;
    thread_args.queue = queue;
    for (int i = 0; i < num_of_breaks + 1; i++) {
        thread_args.output_list[i] = output_list[i];
    }
    

    struct timeval start, end;
    gettimeofday(&start, NULL);


     // Check which operation needs to be performed
    if (!strcmp(operation, "-e")) {
        for (int i = 0; i < num_of_cores - 1; i++) {
            pthread_create(&threads[i], NULL, thread_encrypt, &thread_args);
        }

        // Main process should wait for the threads to complete
        for (int i = 0; i < num_of_cores - 1; i++) {
            pthread_join(threads[i], NULL);
        }
    } else if (!strcmp(operation, "-d")) {
        for (int i = 0; i < num_of_cores - 1; i++) {
            pthread_create(&threads[i], NULL, thread_decrypt, &thread_args);
        }

        // Main process should wait for the threads to complete
        for (int i = 0; i < num_of_cores - 1; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {
        printf("Not a valid operation. Use -e for encryption or -d for decryption.\n");
        return 0;
    }

    gettimeofday(&end, NULL);
    double time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // Print the output
    for (int i = 0; i < num_of_breaks + 1; i++) {
        printf("%s", output_list[i]);
    }

    printf("\nTime Elapsed: %f seconds\n", time_elapsed);

    return 0;
}
