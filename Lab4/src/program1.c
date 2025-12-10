#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "libraries.h"

// static version

void option_1(void)
{
    char *arg1 = strtok(NULL, " \t\n");
    char *arg2 = strtok(NULL, " \t\n");

    size_t length = 0;
    char buffer[1024];

    if (arg1 && arg2)
    {
        int result = gcd(atoi(arg1), atoi(arg2));
        length = snprintf(buffer, 1024, "%d and %d GCD is equals to %d\n", atoi(arg1), atoi(arg2), result);
        write(STDOUT_FILENO, buffer, length);
    }
}

void option_2(void)
{
    char *arg = strtok(NULL, " \t\n");

    size_t length = 0;
    char buffer[1024];

    if (arg)
    {
        float result = e(atoi(arg));
        length = snprintf(buffer, 1024, "Base of natural logarithm with %d steps is equals to %f\n", atoi(arg), result);
        write(STDOUT_FILENO, buffer, length);
    }
}

int main(void)
{
    int bytes = 0;
    char buffer[1024];

    while ((bytes = read(STDIN_FILENO, buffer, 1024 - 1)) > 0)
    {
        buffer[bytes] = 0;

        char *token = strtok(buffer, " \t\n");
        if (!token)
        {
            continue;
        }

        int option = atoi(token);
        switch (option)
        {
        case 1:
        {
            option_1();
            break;
        }
        case 2:
        {
            option_2();
            break;
        }
        default:
            return 1;
        }
    }

    return 0;
}