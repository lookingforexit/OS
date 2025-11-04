#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "hexFuncs.h"

typedef struct
{
    char **numbers;
    uint64_t count;

    char *threadSum;
    uint64_t threadCount;
} ThreadData;

void *calcThreadPartialSum(void *arg)
{
    ThreadData *threadData = arg;
    char *threadSum = strdup("0");
    uint64_t threadCount = 0;

    for (uint64_t i = 0; i < threadData->count; ++i)
    {
        char *sumInThread = hexSum(threadSum, threadData->numbers[i]);
        free(threadSum);
        threadSum = sumInThread;
        ++threadCount;
    }

    threadData->threadSum = threadSum;
    threadData->threadCount = threadCount;

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./main <threads_count> <memory_bytes> <file>\n");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint64_t threadsCount = strtoll(argv[1], NULL, 10);
    uint64_t memoryBytes = strtoll(argv[2], NULL, 10);

    FILE *file = fopen(argv[3], "r");
    if (!file)
    {
        fprintf(stderr, "Error: cannot open file\n");
        return 1;
    }

    uint64_t bytesPerNumber = 32 + 1 + 8;
    uint64_t maxNumbersPerIteration = memoryBytes / bytesPerNumber;

    if (!maxNumbersPerIteration)
    {
        maxNumbersPerIteration = 1;
    }

    char buffer[64];
    char **allNumbers = malloc(sizeof(char*) * maxNumbersPerIteration);

    char *globalSum = strdup("0");
    uint64_t globalCount = 0;

    pthread_t *threads = malloc(sizeof(pthread_t) * threadsCount);
    ThreadData *thread_data = malloc(sizeof(ThreadData) * threadsCount);

    while (1)
    {
        uint64_t totalCount = 0;

        while (totalCount < maxNumbersPerIteration && fgets(buffer, sizeof(buffer), file))
        {
            char *ptr = buffer;
            while (*ptr && *ptr != '\n')
            {
                ++ptr;
            }
            *ptr = 0;

            if (strlen(buffer))
            {
                allNumbers[totalCount] = strdup(buffer);
                ++totalCount;
            }
        }

        if (!totalCount)
        {
            break;
        }

        uint64_t numbersPerThread = totalCount / threadsCount;
        uint64_t remainder = totalCount % threadsCount;

        for (uint64_t i = 0; i < threadsCount; ++i)
        {
            uint64_t startIdx = i * numbersPerThread + (i > 0 ? remainder : 0);
            uint64_t count = numbersPerThread + (i == 0 ? remainder : 0);

            if (count)
            {
                thread_data[i].numbers = allNumbers + startIdx;
                thread_data[i].count = count;
                thread_data[i].threadSum = NULL;
                thread_data[i].threadCount = 0;

                pthread_create(&threads[i], NULL, calcThreadPartialSum, &thread_data[i]);
            }
        }

        for (uint64_t i = 0; i < threadsCount; ++i)
        {
            uint64_t count = numbersPerThread + (i == 0 ? remainder : 0);

            if (count)
            {
                pthread_join(threads[i], NULL);
            }
        }

        for (uint64_t i = 0; i < threadsCount; ++i)
        {
            uint64_t count = numbersPerThread + (i == 0 ? remainder : 0);

            if (count && thread_data[i].threadSum)
            {
                char* newSum = hexSum(globalSum, thread_data[i].threadSum);
                free(globalSum);
                globalSum = newSum;
                free(thread_data[i].threadSum);
                globalCount += thread_data[i].threadCount;
            }
        }

        for (uint64_t i = 0; i < totalCount; ++i)
        {
            free(allNumbers[i]);
        }
    }

    printf("Amount of numbers: %lu\n", globalCount);
    printf("Full sum: %s\n", globalSum);

    if (globalCount)
    {
        char *average = hexDivide(globalSum, globalCount);
        printf("Average floored hex value: %s\n", average);
        free(average);
    }

    free(allNumbers);
    free(globalSum);
    free(threads);
    free(thread_data);
    fclose(file);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %lf seconds\n", elapsed);
    printf("Threads used: %lu\n", threadsCount);

    return 0;
}