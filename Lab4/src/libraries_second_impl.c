#include "libraries.h"

// simple algorithm with for
int gcd(int a, int b)
{
    int res = 1;
    for (int i = 1; i <= (a < b ? a : b); ++i)
    {
        if (!(a % i) && !(b % i))
        {
            res = i;
        }
    }

    return res;
}

// series 1 / n!
float e(int x)
{
    float res = 1.0;
    int fact = 1;

    for (int i = 1; i <= x; ++i)
    {
        fact *= i;
        float value = 1.0 / fact;
        res += value;
    }

    return res;
}