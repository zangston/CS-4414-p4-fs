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

    // Superblock summary
    struct ext2_super_block super_block;
    fseek(file, 1024, SEEK_SET);
    fread(&super_block, sizeof(super_block), 1, file);
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
            super_block.s_blocks_count,
            super_block.s_inodes_count,
            1024 << super_block.s_log_block_size,
            super_block.s_inode_size,
            super_block.s_blocks_per_group,
            super_block.s_inodes_per_group,
            super_block.s_first_ino);

    // Group summary
    int num_groups = super_block.s_blocks_count / super_block.s_blocks_per_group + 1;
    struct ext2_group_desc group;
    for(int i = 0; i < num_groups; i++) {
        fseek(file, 1024 + sizeof(super_block) + i*sizeof(group), SEEK_SET);
        fread(&group, sizeof(group), 1, file);
        int blocks_in_group = super_block.s_blocks_per_group;
        if(i == num_groups - 1) {
            blocks_in_group = super_block.s_blocks_count % super_block.s_blocks_per_group;
        }
        printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i, blocks_in_group, super_block.s_inodes_per_group,
            group.bg_free_blocks_count, group.bg_free_inodes_count, group.bg_block_bitmap,
            group.bg_inode_bitmap, group.bg_inode_table);
    }

    // Free block entries

    
    // Free I-node entries

    
    // I-node summary


    // Directory entries


    // Indirect block references


    // Close .img file
    fclose(file);

    return 0;
}
