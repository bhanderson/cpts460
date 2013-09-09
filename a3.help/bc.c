/*******************************************************
 *                  @bc.c file                          *
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


u16 getblk(u16 blk, char *buf)
{
	readfd( blk/18, ((blk*2)%36)/18, ((blk*2)%36)%18, buf);
}

/*u32 search(INODE *inodePtr, char *name)
{
	DIR *dp = (DIR *) buf3;
	int i,j;
	char *cp, tmp[256];
	for (i = 0; i < 12; i++) {
		cp = buf3;
		dp = (DIR *) buf3;
		getblk(inodePtr->i_block[i], buf3);
		while(cp < buf3 + 1024){
			for (j = 0; j < dp->name_len; j++) {
				tmp[j]=(char)dp->name[j];
			}
			tmp[j] = 0;
			if(strcmp(name, tmp) == 0 ) {
				return dp->inode;
			}else{
				cp += dp->rec_len;
				dp = (DIR *)cp;
				if((dp->inode == 0) && (inodePtr->i_block[i+1] == 0))
					return 0;
			}
		}
	}
	return 0;
}*/

int prints(char *s)
{
	while(*s)
		putc(*s++);
}

gets(char *s)
{
	char c;
	while ( (c = getc()) != '\r'){
		*s++ = c;
		putc(c);
	}
	*s =0;
}

char temp[256];
main()
{
	u16    i,iblk;
	char   c;
	GD    *gp;
	INODE *ip;
	DIR   *dp;
	u32   block;

	prints("read descriptor block #2 into buf1[]\n\r");
	getblk(2, buf1);
	gp = (GD *)buf1;
	iblk = (u16)gp->bg_inode_table;
	prints("inodes blk = "); putc(iblk + '0'); prints("\n\r");

	getblk((u16)iblk, buf1);  // read first inode block block
	ip = (INODE *)buf1 + 1;   // ip->root inode #2

	prints("Enter a filename to boot: ");
	gets(temp);
	prints("\n\r");
	prints(temp);
//	block = search(ip, temp);

	prints("read 0th data block of root inode into buf2[ ]\n\r");

	getblk((u16)ip->i_block[0], buf2);
	dp = (DIR *)buf2;         // buf2 contains DIR entries

	while((char *)dp < &buf2[BLK]){
		c = dp->name[dp->name_len];
		dp->name[dp->name_len] = 0;
		prints(dp->name); putc(' ');
		dp->name[dp->name_len] = c;
		dp = (DIR *)((char *)dp + dp->rec_len);
	}

	prints("\n\rgo?"); getc();
}
