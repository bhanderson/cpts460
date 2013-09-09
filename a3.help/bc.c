/*******************************************************
 *                  @bc.c file                          *
 *                  bryce handerson                     *
 *                  11267615                            *
 *******************************************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#include "ext2.h"
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define BLK 1024
char buf1[BLK], buf2[BLK];

myprints(char *s)
{
	while(*s)
		putc(*s++);
}

mygets(char *s)
{
	char c;
	while ( (c = getc()) != '\r'){
		*s++ = c;
		putc(c);
	}
	*s = 0;
}

u16 getblk(u16 blk, char *buf)
{
	readfd( blk/18, ((blk*2)%36)/18, ((blk*2)%36)%18, buf);
}

u16 find(u16 block, char *name){
	DIR *dirp;
	char *c;
	getblk(block, buf2);
	dirp = (DIR *)buf2;
	while((char *)dirp < &buf2[BLK]){
		c = dirp->name[dirp->name_len];
		dirp->name[dirp->name_len] = 0;
		if (strcmp(dirp->name, name) == 0){
			dirp->name[dirp->name_len] = c;
			return dirp->inode;
		}
		dirp->name[dirp->name_len] = c;
		dirp = (DIR *)((char *)dirp + dirp->rec_len);
	}
	return 0;
}

INODE *gino(u16 blk, u16 node){
	u16 bk, id;
	bk = blk + ((node -1) / 8);
	id = (node - 1) % 8;
	getblk(bk, buf1);

	return (INODE *)buf1 + id;
}

char temp[256];
main()
{
	u16    i,iblk;
	char   c;
	GD    *gp;
	INODE *ip;
	DIR   *dp;
	u32   *bp;

	getblk(2, buf1);
	gp = (GD *)buf1;
	iblk = (u16)gp->bg_inode_table;

	myprints("img: ");
	mygets(temp);

	getblk((u16)iblk, buf1);  // read first inode block block
	ip = (INODE *)buf1 + 1;   // ip->root inode #2
	i = find((u16)ip->i_block[0], "boot");
	// i = 13
	ip = gino(iblk, i);
	i = find((u16)ip->i_block[0], temp);
	// i = 34
	//putc(i+'0');
	//getc();

	ip = gino(iblk, i);

	getblk((u16)ip->i_block[12], buf2);
	setes(0x1000);
	for (i=0; i < 12 && ip->i_block[i] != 0; i ++){
		getblk((u16)ip->i_block[i], (i*1024));
		putc('-');
	}

	for (i=0, bp=buf2; i < 256 && bp[i] > 0; i++){
		getblk((u16)bp[i], (i+12)*1024);
		putc('.');
	}
	getc();
	return 1;/*
	while((char *)dp < &buf2[BLK]){
		c = dp->name[dp->name_len];
		dp->name[dp->name_len] = 0;
		myprints(dp->name); putc(' ');
		dp->name[dp->name_len] = c;
		dp = (DIR *)((char *)dp + dp->rec_len);
	}

	myprints("\n\rgo?"); getc();*/
}
