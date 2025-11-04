#include "hexFuncs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

char* hexSum(const char* n1, const char* n2)
{
    size_t len1 = strlen(n1);
    size_t len2 = strlen(n2);
    size_t maxLen = len1 > len2 ? len1 : len2;
    size_t resultLen = maxLen + 2;

    char *result = malloc(resultLen);
    if (!result)
    {
        char* msg = "Error: unable to allocate memory\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return NULL;
    }

    uint8_t carry = 0;
    uint64_t writePos = resultLen - 2;
    result[resultLen - 1] = 0;

    for (size_t i = 0; i < maxLen; ++i)
    {
        uint8_t digit1 = 0, digit2 = 0;

        if (i < len1)
        {
            char c = n1[len1 - 1 - i];
            digit1 = isdigit(c) ? c - '0' : c - 'a' + 10;
        }

        if (i < len2)
        {
            char c = n2[len2 - 1 - i];
            digit2 = isdigit(c) ? c - '0' : c - 'a' + 10;
        }

        uint8_t sum = digit1 + digit2 + carry;
        carry = sum / 16;
        sum %= 16;

        result[writePos--] = sum < 10 ? sum + '0' : sum - 10 + 'a';
    }

    if (carry)
    {
        result[writePos--] = carry < 10 ? carry + '0' : carry - 10 + 'a';
    }

    char *finalResult = malloc(strlen(result + writePos + 1) + 1);
    if (!finalResult)
    {
        char* msg = "Error: unable to allocate memory\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        free(result);
        return NULL;
    }

    strcpy(finalResult, result + writePos + 1);
    free(result);
    return finalResult;
}

char* hexDivide(const char* hex, uint64_t divisor) // like floor approximation
{
    if (divisor == 0)
    {
        return NULL;
    }

    if (!strcmp(hex, "0"))
    {
        return strdup("0");
    }

    size_t len = strlen(hex);
    char *result = malloc(len + 1);
    if (!result)
    {
        char* msg = "Error: unable to allocate memory\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return NULL;
    }

    uint64_t remainder = 0;
    size_t resultPos = 0;
    for (size_t i = 0; i < len; ++i)
    {
        uint8_t digit = isdigit(hex[i]) ? hex[i] - '0' : hex[i] - 'a' + 10;
        remainder = remainder * 16 + digit;

        uint64_t quotient = remainder / divisor;
        remainder = remainder % divisor;

        if (quotient > 0 || resultPos > 0)
        {
            result[resultPos++] = quotient < 10 ? quotient + '0' : quotient - 10 + 'a';
        }
    }

    if (resultPos == 0)
    {
        result[resultPos++] = '0';
    }
    result[resultPos] = 0;

    return result;
}