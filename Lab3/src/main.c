#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

#define SHM_NAME "shared_memory"
#define SEM_DATA_RDY "sem_data"
#define SEM_RESPONSE_RDY "sem_response"
#define SHM_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        return 1;
    }

    char filename[256];
    const char msg[] = "Input the name of file: ";
    write(STDOUT_FILENO, msg, sizeof(msg));
    ssize_t bytes = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    if (bytes <= 0)
    {
        const char error_msg[] = "Error: unable to get user data\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }
    filename[bytes - 1] = '\0';

    shm_unlink(SHM_NAME);
    sem_unlink(SEM_DATA_RDY);
    sem_unlink(SEM_RESPONSE_RDY);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        const char error_msg[] = "Error: unable to create shared memory\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1)
    {
        const char error_msg[] = "Error: unable to truncate shared memory\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }

    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        const char error_msg[] = "Error: unable to mmap shared memory\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }

    sem_t *sem_data_rdy = sem_open(SEM_DATA_RDY, O_CREAT, 0666, 0);
    sem_t *sem_response_rdy = sem_open(SEM_RESPONSE_RDY, O_CREAT, 0666, 0);

    if (sem_data_rdy == SEM_FAILED || sem_response_rdy == SEM_FAILED)
    {
        const char error_msg[] = "Error: unable to create semaphores\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }

    pid_t child_id = fork();
    switch (child_id)
    {
    case -1:
    {
        const char error_msg[] = "Error: unable to create child process\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }
    case 0:
    {
        int exec_status = execl("./child", "child", NULL);
        if (exec_status == -1)
        {
            const char error_msg[] = "Error: unable to execl\n";
            write(STDERR_FILENO, error_msg, sizeof(error_msg));
            exit(EXIT_FAILURE);
        }
    }
    default:
    {
        strncpy(shm_ptr, filename, SHM_SIZE - 1);
        shm_ptr[SHM_SIZE - 1] = '\0';
        sem_post(sem_data_rdy);

        const char msg[] = "Input numbers (q for quit):\n";
        write(STDOUT_FILENO, msg, sizeof(msg));

        char buffer[128];
        int bytes;
        while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytes] = '\0';

            strncpy(shm_ptr, buffer, SHM_SIZE - 1);
            shm_ptr[SHM_SIZE - 1] = '\0';
            sem_post(sem_data_rdy);

            sem_wait(sem_response_rdy);

            char buffer2[128];
            strncpy(buffer2, shm_ptr, sizeof(buffer2) - 1);
            buffer2[sizeof(buffer2) - 1] = '\0';

            if (buffer2[0] == '1')
            {
                const char msg[] = "Done\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                break;
            }
        }

        int exec_child_status;
        wait(&exec_child_status);

        if (WIFEXITED(exec_child_status))
        {
            if (WEXITSTATUS(exec_child_status) != 0)
            {
                const char error_msg[] = "Error: child process terminated with error\n";
                write(STDERR_FILENO, error_msg, sizeof(error_msg));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            const char error_msg[] = "Error: the process is executing\n";
            write(STDERR_FILENO, error_msg, sizeof(error_msg));
            exit(EXIT_FAILURE);
        }

        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        shm_unlink(SHM_NAME);
        sem_close(sem_data_rdy);
        sem_close(sem_response_rdy);
        sem_unlink(SEM_DATA_RDY);
        sem_unlink(SEM_RESPONSE_RDY);

        exit(EXIT_SUCCESS);
    }
    }

    return 0;
}
