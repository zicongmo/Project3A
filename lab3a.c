/*
NAME: Zicong Mo, Benjamin Yang
ID: 804654167, 904771533
EMAIL: josephmo1594@ucla.edu, byang77@ucla.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>	
#include "ext2_fs.h"

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "Error: Must include file system image\n");
		exit(1);
	}

	int fd = open(argv[1], O_RDONLY);
	if(fd < 0){
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(2);
	}

	/* 1024 bytes */
	struct ext2_super_block super;
	lseek(fd, 1024, SEEK_SET); /* Put file offeset at 1024 */
	int r = read(fd, &super, sizeof(super));
	if(r < 0){
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(2);
	}
	if(super.s_magic != EXT2_SUPER_MAGIC){
		fprintf(stderr, "Error: Specified file is not EXT2\n");
		exit(2);
	}

	printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
		   super.s_blocks_count,
		   super.s_inodes_count,
		   (EXT2_MIN_BLOCK_SIZE << super.s_log_block_size),
		   super.s_inode_size,
		   super.s_blocks_per_group,
		   super.s_inodes_per_group,
		   super.s_first_ino);

	/* 32 bytes */
	struct ext2_group_desc block_group;
	r = read(fd, &block_group, sizeof(block_group));
	if(r < 0){
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(2);
	}
	printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",
		   0,
		   super.s_blocks_count,
		   super.s_inodes_count,
		   block_group.bg_free_blocks_count,
		   block_group.bg_free_inodes_count,
		   block_group.bg_block_bitmap,
		   block_group.bg_inode_bitmap,
		   block_group.bg_inode_table
		   );
	exit(0);
}
