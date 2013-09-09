typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#include "ext2.h"
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define BLK 1024
char buf1[BLK], buf2[BLK];
char temp[256];

u16 getblk(u16 blk, char *buf)
{
	readfd( blk/18, ((blk*2)%36)/18, ((blk*2)%36)%18, buf);
}

int prints(char *s)
{
	while(*s)
		putc(*s++);
}

int gets(char *s)
{
	char c;
	while ( (c = getc()) != '\r'){
		*s++ = c;
		putc(c);
	}
	*s =0;
}


u32 search(INODE *iPtr, char *name)
{
	DIR *dirp;
	char c;
	u32		node = 0;
	getblk((u16)iPtr->i_block[0], buf2);
	dirp = (DIR *)buf2;

	while((char *)dirp < &buf2[BLK]){
		c = dirp->name[dirp->name_len];
		dirp->name[dirp->name_len] = 0;
		prints(dirp->name); putc(' ');
		if (strcmp(name, dirp->name) == 0 ){
			dirp->name[dirp->name_len] = c;
			node = dirp->inode;
			break;
		} else {
			dirp->name[dirp->name_len] = c;
			dirp = (DIR *)((char *)dirp + dirp->rec_len);
		}
	}
	return node;
}
main()
{
	u16		iblk;
	INODE	*ip;
	GD		*gp;
	DIR		*dp;
	u32 ino=0;


	getblk(2, buf1); // read descriptor block #w into buf1
	gp = (GD *)buf1;
	iblk = (u16)gp->bg_inode_table;
	prints("inodes blk = "); putc(iblk + '0'); prints("\n\r");

	getblk((u16)iblk, buf1); // read first inode block
	ip = (INODE *)buf1 + 1;  // ip->root inode #2

	// get boot inode
	prints("enter a filename to boot: ");
	gets(temp);
	prints("\n\r");

	ino = search(ip, "boot");
	ip = (INODE *)buf1 + ino -1;
	ino = search(ip, temp);
	ip = (INODE *)buf1 + ino -1;
	getblk((u16)ip->i_block[0],0x1000);
	return 1;
}
