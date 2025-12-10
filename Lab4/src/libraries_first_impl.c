#include "libraries.h"

#include <math.h>

// Euclid algorithm
int gcd(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }

    return a;
}

// (1 + 1/x)^x
float e(int x)
{
    return powf(1.0 + 1.0 / x, x);
}