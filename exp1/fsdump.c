#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <time.h>
#include <sys/stat.h>

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

    int EXT2_BLOCK_SIZE = 1024;

    struct ext2_super_block super_block;
    fseek(file, EXT2_BLOCK_SIZE, SEEK_SET);
    fread(&super_block, sizeof(super_block), 1, file);
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
            super_block.s_blocks_count,
            super_block.s_inodes_count,
            EXT2_BLOCK_SIZE << super_block.s_log_block_size,
            super_block.s_inode_size,
            super_block.s_blocks_per_group,
            super_block.s_inodes_per_group,
            super_block.s_first_ino);

    // Group summary
    int num_groups = super_block.s_blocks_count / super_block.s_blocks_per_group + 1;
    struct ext2_group_desc group;
    for(int i = 0; i < num_groups; i++) 
    {
        fseek(file, 1024 + sizeof(super_block) + i*sizeof(group), SEEK_SET);
        fread(&group, sizeof(group), 1, file);
        int blocks_in_group = super_block.s_blocks_per_group;
        if(i == num_groups - 1) 
        {
            blocks_in_group = super_block.s_blocks_count % super_block.s_blocks_per_group;
        }
        printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i, blocks_in_group, super_block.s_inodes_per_group,
            group.bg_free_blocks_count, group.bg_free_inodes_count, group.bg_block_bitmap,
            group.bg_inode_bitmap, group.bg_inode_table);
    }

    // Free block entries
    // Scan free block bitmaps
    for (int i = 0; i < num_groups; i++) 
    {
        // Seek to free block bitmap
        fseek(file, EXT2_BLOCK_SIZE * (group.bg_block_bitmap + i * super_block.s_blocks_per_group / 8), SEEK_SET);

        // Read in free block bitmap
        char *bitmap = malloc(super_block.s_blocks_per_group / 8);
        fread(bitmap, super_block.s_blocks_per_group / 8, 1, file);

        // Print out free blocks
        for (int j = 0; j < (int) super_block.s_blocks_per_group; j++) 
        {
            int byte = j / 8;
            int bit = j % 8;
            if ((bitmap[byte] & (1 << bit)) == 0) 
            {
                printf("BFREE,%d\n", i * super_block.s_blocks_per_group + j + 1);
            }
        }

    free(bitmap);
    }
    
    // Free I-node entries
    for(int i = 0; i < num_groups; i++) 
    {
        // Seek to the beginning of the i-node bitmap for this group
        fseek(file, group.bg_inode_bitmap * EXT2_BLOCK_SIZE, SEEK_SET);

        // Read in the i-node bitmap
        __u8 inode_bitmap[super_block.s_inodes_per_group / 8];
        fread(inode_bitmap, sizeof(__u8), super_block.s_inodes_per_group / 8, file);

        // Loop through each bit in the bitmap
        for(int j = 0; j < (int) super_block.s_inodes_per_group; j++) 
        {
            // Calculate the byte and bit indices for this inode
            int byte_index = j / 8;
            int bit_index = j % 8;

            // Check if the bit is set
            if((inode_bitmap[byte_index] & (1 << bit_index)) == 0) 
            {
                // Bit is not set, inode is free
                printf("IFREE,%d\n", i * super_block.s_inodes_per_group + j + 1);
            }
        }
    }
    
    // I-node summary
    for (int i = 0; i < num_groups; i++) 
    {
        // Seek to the beginning of the i-node table for this group
        fseek(file, group.bg_inode_table * EXT2_BLOCK_SIZE, SEEK_SET);

        // Loop through each i-node in the table
        for (int j = 0; j < (int) super_block.s_inodes_per_group; j++) {
            struct ext2_inode inode;
            fread(&inode, sizeof(inode), 1, file);

            if (inode.i_mode != 0 && inode.i_links_count != 0) 
            {
                char file_type = '?';
                if (S_ISDIR(inode.i_mode)) 
                {
                    file_type = 'd';
                } else if (S_ISREG(inode.i_mode)) 
                {
                    file_type = 'f';
                } else if (S_ISLNK(inode.i_mode)) 
                {
                    file_type = 'l';
                }
                printf("INODE,%d,%c,%o,%d,%d,%d,", i * super_block.s_inodes_per_group + j + 1, file_type, inode.i_mode & 0x0FFF,
                    inode.i_uid, inode.i_gid, inode.i_links_count);

                char ctime[20];
                char mtime[20];
                char atime[20];

                time_t ctime_raw = inode.i_ctime;
                time_t mtime_raw = inode.i_mtime;
                time_t atime_raw = inode.i_atime;

                strftime(ctime, 20, "%m/%d/%y %H:%M:%S", localtime(&ctime_raw));
                strftime(mtime, 20, "%m/%d/%y %H:%M:%S", localtime(&mtime_raw));
                strftime(atime, 20, "%m/%d/%y %H:%M:%S", localtime(&atime_raw));

                printf("%s,%s,%s,", ctime, mtime, atime);

                printf("%d,%d", inode.i_size, inode.i_blocks / 2);
                for (int k = 0; k < 15; k++) 
                {
                    printf(",%d", inode.i_block[k]);
                }
                printf("\n");
            }
        }
    }

    // Close .img file
    fclose(file);

    return 0;
}
