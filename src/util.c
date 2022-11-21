/**
 * General utilities
*/
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Clear stdin buffer
*/
void clear_stdin_buff() {
    char ch;
    while ((ch = getchar()) != EOF && ch != '\n');
}