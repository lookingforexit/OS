#ifndef CHILD_H
#define CHILD_H

typedef enum
{
    OK,
    NOT_OK
} status_code;

status_code check_positive_composite(const int number);

#endif