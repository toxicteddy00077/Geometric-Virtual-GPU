#include "common.h"

FILE* FILE_CHECK(const char*filename, const char *mode){
    FILE *file = fopen(filename, mode);
    if(!file){
        printf("Error: Could not open file\n");
        return NULL;
    }
    return file;
}

