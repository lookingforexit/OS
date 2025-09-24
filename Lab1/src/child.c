#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../include/child.h"

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

int main(int argc, char* argv[])
{
    if (argc != 1)
    {
        return 1;
    }

    char filename[256];
    ssize_t status = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    if (status < 0) 
    {
        const char error_msg[] = "Error: unable to read filename\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        return 1;
    }
    filename[status] = '\0';

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
        ssize_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (bytes <= 0)
        {
            const char error_msg[] = "Error: unable to read info from parent\n";
        }
        buffer[bytes] = '\0';

        if (buffer[0] == 'q')
        {
            const char error_msg[] = "1";
            write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
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

            const char msg[] = "0";
            write(STDOUT_FILENO, msg, sizeof(msg));
        }
        else
        {
            const char error_msg[] = "1";
            write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
            break;
        }
    }
    
    close(file);
    return 0;    
}