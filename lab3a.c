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
#include <time.h>
#include "ext2_fs.h"

char time_string[64];

char get_file_type(__u16 i_mode){
	switch(i_mode & 0xF000){
		case 0xA000:
			return 's';
		case 0x8000:
			return 'f';
		case 0x4000:
			return 'd';
		default:
			return '?';
	}
}

void get_gmt_time(__u32 time){
	time_t timer = (time_t) time;
	struct tm* info;
	info = gmtime(&timer);
	sprintf(time_string, "%02d/%02d/%02d %02d:%02d:%02d", 
		    info->tm_mon+1,
		    info->tm_mday,
		    (info->tm_year%100),
		    info->tm_hour,
		    info->tm_min,
		    info->tm_sec);
}

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
	int r = pread(fd, &super, sizeof(super), 1024);
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
	r = pread(fd, &block_group, sizeof(block_group), 1024 + sizeof(super));
	if(r < 0){
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(2);
	}
	/* TODO: Check this */
	printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",
		   0,
		   super.s_blocks_count,
		   super.s_inodes_count,
		   block_group.bg_free_blocks_count,
		   block_group.bg_free_inodes_count,
		   block_group.bg_block_bitmap,
		   block_group.bg_inode_bitmap,
		   block_group.bg_inode_table);

	/* Free blocks */
	char* bitmap = malloc(EXT2_MIN_BLOCK_SIZE);
	pread(fd, (void*) bitmap, EXT2_MIN_BLOCK_SIZE,
		  EXT2_MIN_BLOCK_SIZE * block_group.bg_block_bitmap);
	__u32 i;
	int mask = 1;
	for(i = 0; i < super.s_blocks_per_group; i++){
		int bit = (bitmap[i/8] & (mask << (i%8))) >> (i%8);
		if(bit == 0){
			printf("BFREE,%u\n", i+1);
		}
	}

	/* Free inode */
	pread(fd, (void*) bitmap, EXT2_MIN_BLOCK_SIZE,
		  EXT2_MIN_BLOCK_SIZE * block_group.bg_inode_bitmap);
	for(i = 0; i < super.s_inodes_per_group; i++){
		int bit = (bitmap[i/8] & (mask << (i%8))) >> (i%8);
		if(bit == 0){
			printf("IFREE,%u\n", i+1);
		}
	}

	/* Inode table */
	struct ext2_inode* inode_table = malloc(sizeof(struct ext2_inode) * super.s_inodes_per_group);
	pread(fd, (void*) inode_table, sizeof(struct ext2_inode) * super.s_inodes_per_group,
		  EXT2_MIN_BLOCK_SIZE * block_group.bg_inode_table);	
	for(i = 0; i < super.s_inodes_per_group; i++){
		struct ext2_inode inode = inode_table[i];	
		int bit = (bitmap[i/8] & (mask << (i%8))) >> (i%8);
		if(bit == 1 && (inode.i_mode != 0) && (inode.i_links_count != 0)){
			printf("INODE,%u,%c,%o,%u,%u,%u,",
				   i+1,
				   get_file_type(inode.i_mode),
				   inode.i_mode & 0x0FFF, 
				   inode.i_uid, 
				   inode.i_gid, 
				   inode.i_links_count);
			/* TODO: Check this */
			get_gmt_time(inode.i_ctime);
			printf("%s,", time_string);
			get_gmt_time(inode.i_mtime);
			printf("%s,", time_string);
			get_gmt_time(inode.i_atime);
			printf("%s,%u,%u,", time_string, inode.i_size, inode.i_blocks);
			int j;
			for(j = 0; j < EXT2_N_BLOCKS; j++){
				if(j == EXT2_N_BLOCKS - 1){
					printf("%u\n", inode.i_block[j]);
				}
				else{
					printf("%u,", inode.i_block[j]);
				}
			}
			/* TODO: Scan every non-zerp data block */
			if(get_file_type(inode.i_mode) == 'd'){
				struct ext2_dir_entry* dir_entry = malloc(sizeof(struct ext2_dir_entry));
				pread(fd, (void*) dir_entry, sizeof(struct ext2_dir_entry),
					  EXT2_MIN_BLOCK_SIZE * inode.i_block[0]);
				printf("DIRENT,%u,%u,%u,%u,%u,%s\n",
					   i+1, 
						,
					   dir_entry->inode,
					   dir_entry->rec_len,
					   dir_entry->name_len,
					   dir_entry->name);
			}
		}	

	}

	free(inode_table);
	free(bitmap);
	exit(0);
}
