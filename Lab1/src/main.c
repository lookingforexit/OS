#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char* argv[])
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
    filename[bytes-1] = '\0';

    int parent_to_child[2];
    if (pipe(parent_to_child) == -1)
    {
        const char error_msg[] = "Error: unable to create parent_to_child pipe\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }

    int child_to_parent[2];
    if (pipe(child_to_parent) == -1)
    {
        const char error_msg[] = "Error: unable to create child_to_parent pipe\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        exit(EXIT_FAILURE);
    }

    pid_t child_id = fork();
    switch (child_id)
    {
        case -1:
            const char error_msg[] = "Error: unable to create child process\n";
            write(STDERR_FILENO, error_msg, sizeof(error_msg));
            exit(EXIT_FAILURE);
        case 0:
            close(parent_to_child[1]);
            dup2(parent_to_child[0], STDIN_FILENO);
            close(parent_to_child[0]);

            close(child_to_parent[0]);
            dup2(child_to_parent[1], STDOUT_FILENO);
            close(child_to_parent[1]);

            int exec_status = execl("./child", "child", NULL);
            if (exec_status == -1)
            {
                const char error_msg[] = "Error: unable to execl\n";
                write(STDERR_FILENO, error_msg, sizeof(error_msg));
                exit(EXIT_FAILURE);
            }
        default:
            close(parent_to_child[0]);
            close(child_to_parent[1]);

            write(parent_to_child[1], filename, strlen(filename) + 1);

            const char msg[] = "Input numbers (q for quit):\n";
            write(STDOUT_FILENO, msg, sizeof(msg));

            char buffer[128];
            int bytes;
            while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytes] = '\0';

                write(parent_to_child[1], buffer, bytes);

                char buffer2[128];
                ssize_t bytes2 = read(child_to_parent[0], buffer2, sizeof(buffer2) - 1);
                if (bytes2 <= 0)
                {
                    const char error_msg[] = "Error: unable to read child info\n";
                    write(STDERR_FILENO, error_msg, sizeof(error_msg));
                    break;
                }
                buffer2[bytes2] = '\0';

                if (buffer2[0] == '1')
                {
                    const char msg[] = "Done\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    break;
                }
            }

            close(parent_to_child[1]);
            close(child_to_parent[0]);

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
                exit(EXIT_SUCCESS);
            }
            else
            {
                const char error_msg[] = "Error: the process is executing\n";
                write(STDERR_FILENO, error_msg, sizeof(error_msg));
                exit(EXIT_FAILURE);
            }
    }

    return 0;
}