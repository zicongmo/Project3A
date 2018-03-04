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
		exit(1);
	}

	/* Get the superblock */
	struct ext2_super_block super;
	lseek(fd, 1024, SEEK_SET); /* Put file offeset at 1024 */
	int r = read(fd, &super, sizeof(super));
	if(r < 0){
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
	if(super.s_magic != EXT2_SUPER_MAGIC){
		fprintf(stderr, "Error: Specified file is not EXT2\n");
		exit(1);
	}

	printf("Num inodes: %u\n", super.s_inodes_count);
	exit(0);
}
