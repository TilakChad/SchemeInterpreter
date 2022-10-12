#pragma once 

#include <stdio.h>
#include <stdlib.h> 

#if defined(__GNUC__)
#define Unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define Unreachable() __assume(false);
#endif

#define Unimplemented()                                                                                                \
    {                                                                                                                  \
        fprintf(stderr, "Function %s() not implemented : %d.", __func__, __LINE__);                                    \
        abort();                                                                                                       \
    }
#define Assert(str)                                                                                                    \
    {                                                                                                                  \
        if (!(str))                                                                                                    \
        {                                                                                                              \
            fprintf(stderr, "Function : %s() Line : %d Assertion Failed : %s.\n", __func__, __LINE__, #str);           \
            abort();                                                                                                   \
        }                                                                                                              \
    }
