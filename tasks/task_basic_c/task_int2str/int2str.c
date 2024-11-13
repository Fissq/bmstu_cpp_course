#include "int2str.h"
#include <stdlib.h>
#include <stdio.h>

char* int2str(int number) {
    char* str = malloc(13);
    
    if (!str) {
        return NULL;
    }

    int i = 12; 
    unsigned int absolute = (number < 0) ? -number : number;

    str[i] = '\0';

    if (number == 0) {
        str[--i] = '0';
    } else {
        while (absolute > 0) {
            str[--i] = absolute % 10 + '0';
            absolute /= 10;
        }
    }

    if (number < 0) {
        str[--i] = '-';
    }

    return &str[i];
}