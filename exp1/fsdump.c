#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include "ext2_fs.h"

int main(int argc, char *argv[])
{
    // Check if a file has been specified in execution command
    if(argc < 2)
    {
        printf("Error: no filename supplied\n");
        printf("Expected command: %s name\n", argv[0]);
        return 1;
    }

    // Read filename from command line args
    char *filename = argv[1];

    // Attempt to open file
    FILE *file = fopen(filename, "rb");

    // Throw error if file not found
    if(file == NULL)
    {
        printf("Error: File %s not found\n", argv[1]);
        return 1;
    }

    

    // Close .img file
    fclose(file);

    return 0;
}
