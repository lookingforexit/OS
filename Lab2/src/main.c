#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include "hexFuncs.h"

typedef struct
{
    FILE *file;
    pthread_mutex_t *fileMutex;
    int8_t *isEOF;

    char **fullSum;
    pthread_mutex_t *sumMutex;

    uint64_t *totalNumbersCount;
    uint64_t numbersPerChunk;

    uint64_t threadsCount;
} ThreadData;

void *calcThreadPartialSum(void *arg)
{
    ThreadData *threadData = arg;
    char buffer[64]; // approximation for each number size

    while (1)
    {
        pthread_mutex_lock(threadData->fileMutex);
        if (*threadData->isEOF)
        {
            pthread_mutex_unlock(threadData->fileMutex);
            break;
        }

        uint64_t localNumbersCount = 0;
        char *threadSum = strdup("0");

        for (uint64_t i = 0; i < threadData->numbersPerChunk; ++i)
        {
            if (fgets(buffer, sizeof(buffer), threadData->file))
            {
                char *ptr = buffer;
                while (*ptr && *ptr != '\n')
                {
                    ++ptr;
                }
                *ptr = 0;

                if (!strlen(buffer))
                {
                    --i;
                    continue;
                }

                char *sumInThread = hexSum(threadSum, buffer);
                free(threadSum);
                threadSum = sumInThread;

                ++localNumbersCount;
            }
            else
            {
                *threadData->isEOF = 1;
                break;
            }
        }
        pthread_mutex_unlock(threadData->fileMutex);

        if (!localNumbersCount)
        {
            free(threadSum);
            break;
        }

        pthread_mutex_lock(threadData->sumMutex);
        char *newFullSum = hexSum(threadSum, *threadData->fullSum);
        free(*(threadData->fullSum));
        *(threadData->fullSum) = newFullSum;
        *(threadData->totalNumbersCount) += localNumbersCount;
        pthread_mutex_unlock(threadData->sumMutex);

        free(threadSum);
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./main <threads_count> <memory_kb> <file>\n");
        return 1;
    }

    uint64_t threadsCount = strtoll(argv[1], NULL, 10);
    uint64_t memoryKB = strtoll(argv[2], NULL, 10);

    uint64_t memoryBytes = memoryKB * 1024;
    uint64_t numbersPerChunk = memoryBytes / threadsCount / 35; // approximation for numbers-on-thread

    FILE *file = fopen(argv[3], "r");
    if (!file)
    {
        fprintf(stderr, "Error: cannot open file\n");
        return 1;
    }

    pthread_mutex_t fileMutex;
    pthread_mutex_init(&fileMutex, NULL);
    pthread_mutex_t sumMutex;
    pthread_mutex_init(&sumMutex, NULL);

    char *global_sum = strdup("0");
    uint64_t global_count = 0;
    int8_t file_ended = 0;

    pthread_t *threads = malloc(sizeof(pthread_t) * threadsCount);
    ThreadData *thread_data = malloc(sizeof(ThreadData) * threadsCount);

    for (uint64_t i = 0; i < threadsCount; ++i)
    {
        thread_data[i].file = file;
        thread_data[i].fileMutex = &fileMutex;
        thread_data[i].sumMutex = &sumMutex;
        thread_data[i].fullSum = &global_sum;
        thread_data[i].totalNumbersCount = &global_count;
        thread_data[i].numbersPerChunk = numbersPerChunk;
        thread_data[i].isEOF = &file_ended;
        thread_data[i].threadsCount = threadsCount;
        pthread_create(&threads[i], NULL, calcThreadPartialSum, &thread_data[i]);
    }

    for (uint64_t i = 0; i < threadsCount; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    printf("Amount of numbers: %lu\n", global_count);
    printf("Full sum: %s\n", global_sum);

    if (global_count > 0)
    {
        char *average = hexDivide(global_sum, global_count);
        printf("Average floored hex value: %s\n", average);
        free(average);
    }

    free(global_sum);
    free(threads);
    free(thread_data);
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&sumMutex);
    fclose(file);

    return 0;
}