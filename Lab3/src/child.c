#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define SHM_NAME "shared_memory"
#define SEM_DATA_RDY "sem_data"
#define SEM_RESPONSE_RDY "sem_response"
#define SHM_SIZE 1024

typedef enum
{
    OK = 0,
    NOT_OK = 1
} status_code;

status_code check_positive_composite(const int number)
{
    if (number == 1)
    {
        return OK;
    }

    for (int i = 2; i * i <= number; ++i)
    {
        if (number % i == 0)
        {
            return OK;
        }
    }

    return NOT_OK;
}

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        return 1;
    }

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        const char error_msg[] = "Error: unable to open shared memory\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        return 1;
    }

    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        const char error_msg[] = "Error: unable to mmap shared memory\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        return 1;
    }

    sem_t *sem_data_rdy = sem_open(SEM_DATA_RDY, 0);
    sem_t *sem_response_rdy = sem_open(SEM_RESPONSE_RDY, 0);

    if (sem_data_rdy == SEM_FAILED || sem_response_rdy == SEM_FAILED)
    {
        const char error_msg[] = "Error: unable to open semaphores\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        return 1;
    }

    sem_wait(sem_data_rdy);
    char filename[256];
    strncpy(filename, shm_ptr, sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';

    int file = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (file == -1)
    {
        const char error_msg[] = "Error: unable to open file\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        return 1;
    }

    char buffer[32];
    while (1)
    {
        sem_wait(sem_data_rdy);
        strncpy(buffer, shm_ptr, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        if (buffer[0] == 'q')
        {
            strncpy(shm_ptr, "1", SHM_SIZE - 1);
            shm_ptr[SHM_SIZE - 1] = '\0';
            sem_post(sem_response_rdy);
            break;
        }

        char *endptr = NULL;
        int number = strtol(buffer, &endptr, 10);

        if (endptr == buffer || (*endptr != '\n' && *endptr != '\0'))
        {
            break;
        }

        status_code status = check_positive_composite(number);

        if (status == OK)
        {
            int i = 0;
            int num_copy = number;
            char number_buffer[32];

            do
            {
                number_buffer[i++] = num_copy % 10 + '0';
                num_copy /= 10;
            } while (num_copy > 0);

            int start = 0;
            int end = i - 1;

            while (start < end)
            {
                char temp = number_buffer[start];
                number_buffer[start] = number_buffer[end];
                number_buffer[end] = temp;
                ++start;
                --end;
            }

            number_buffer[i++] = '\n';
            write(file, number_buffer, i);

            strncpy(shm_ptr, "0", SHM_SIZE - 1);
            shm_ptr[SHM_SIZE - 1] = '\0';
            sem_post(sem_response_rdy);
        }
        else
        {
            strncpy(shm_ptr, "1", SHM_SIZE - 1);
            shm_ptr[SHM_SIZE - 1] = '\0';
            sem_post(sem_response_rdy);
            break;
        }
    }

    close(file);

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_data_rdy);
    sem_close(sem_response_rdy);

    return 0;
}
