#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include "libraries.h"

// dynamic version

typedef int (*gcd_ptr)(int, int);
typedef float (*exp_ptr)(int);

enum Libraries
{
    FIRST = 0,
    SECOND
};

int option_0(const char **libraries, void **library, int *current_library,
             gcd_ptr *gcd, exp_ptr *e)
{
    dlclose(*library);

    switch (*current_library)
    {
    case FIRST:
    {
        *current_library = SECOND;
        break;
    }
    case SECOND:
    {
        *current_library = FIRST;
        break;
    }
    default:
        return 1;
    }

    char buffer[1024];

    *library = dlopen(libraries[*current_library], RTLD_LAZY);
    if (!*library)
    {
        int length = snprintf(buffer, 1024, "Error: invalid switch libs; %s\n", dlerror());
        write(STDERR_FILENO, buffer, length);
        return 1;
    }

    *gcd = dlsym(*library, "gcd");
    if (!gcd)
    {
        const char msg[] = "Error: unable to get gcd impl\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
    }

    *e = dlsym(*library, "e");
    if (!e)
    {
        const char msg[] = "Error: unable to get e impl\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
    }

    int length = snprintf(buffer, 1024, "Switched to %s library\n", libraries[*current_library]);
    write(STDOUT_FILENO, buffer, length);

    return 0;
}

void option_1(gcd_ptr gcd)
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

void option_2(exp_ptr e)
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
    const char *libraries[] = {"./libd1.so", "./libd2.so"};
    char buffer[1024];

    int current_library = FIRST;
    gcd_ptr gcd = NULL;
    exp_ptr e = NULL;

    void *library = dlopen(libraries[current_library], RTLD_LAZY);
    if (!library)
    {
        int length = snprintf(buffer, 1024, "Error: unable to load lib; %s\n", dlerror());
        write(STDERR_FILENO, buffer, length);
        return 1;
    }

    gcd = dlsym(library, "gcd");
    if (!gcd)
    {
        const char msg[] = "Error: unable to get gcd impl\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
    }

    e = dlsym(library, "e");
    if (!e)
    {
        const char msg[] = "Error: unable to get e impl\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
    }

    int bytes = 0;
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
        case 0:
        {
            int result = option_0(libraries, &library, &current_library, &gcd, &e);
            if (result)
            {
                return result;
            }
            break;
        }
        case 1:
        {
            option_1(gcd);
            break;
        }
        case 2:
        {
            option_2(e);
            break;
        }
        default:
            return 1;
        }
    }

    if (library)
    {
        dlclose(library);
    }

    return 0;
}