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

int mygets(char *s)
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
	DIR *dirp = (DIR*) buf2;
	char c;
	getblk((u16)iPtr->i_block[0], buf2);

	while((char *)dirp < &buf2[BLK]){
		c = dirp->name[dirp->name_len];
		dirp->name[dirp->name_len] = 0;
		if (strcmp(name, dirp->name) == 0 ){
			dirp->name[dirp->name_len] = c;
			return dirp->inode;
		} else {
			dirp->name[dirp->name_len] = c;
			dirp = (DIR *)((char *)dirp + dirp->rec_len);
		}
	}
	return 0;
}
main()
{
	u16		i,iblk;
	INODE	*ip;
	GD		*gp;
	u32 	ino;
	u32		*pino;

	getblk(2, buf1); // read descriptor block #w into buf1
	gp = (GD *)buf1;
	iblk = (u16)gp->bg_inode_table;

	getblk((u16)iblk, buf1); // read first inode block
	ip = (INODE *)buf1 + 1;  // ip->root inode #2

	// get boot inode
	prints("enter a filename to boot: ");
	mygets(temp);


	ino = search(ip, "boot");

	getblk(ino/8+iblk,buf1);
	ip = (INODE *)buf1 + (ino-1)%8;
	ino = search(ip, temp);
	getblk(ino/8+iblk,buf1);
	ip = (INODE *)buf1 + (ino-1)%8;
	getblk((u16)ip->i_block[12],buf2);
	setes(0x1000);

	for (i=0; i < 12; i++){
		getblk((u16)ip->i_block[i], BLK*i ); putc('.');
		inces();
	}
	pino = (u32 *)buf2;
	while(*pino != 0){
		getblk((u16)*pino,BLK*(i));
		ino++;
		i++;
	}
	return 1;
}
