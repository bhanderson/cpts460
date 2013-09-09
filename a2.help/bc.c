/******************** Assume:*********************
  FD is an EXT2 FS for MTX to run
  FD contains this booter in block0
  mtx kernel image begins at block 1000
 *************************************************/

typedef unsigned char  u8;
typedef unsigned short u16;

#define BLK 1024

char buf[BLK];
char ans[64];

int prints(char *s)
{
	while(*s)
		putc(*s++);
}

int getblk(u16 blk, char *buf)
{
	readfd( blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
}

main()
{
	u16 i, nblk;

	myprints("Enter a filename to boot: ");
	mygets(ans);
	prints("\n\r");
	if (strcmp(ans, "quit")==0)
		break;
	}

	prints("boot mtx\n\r"); getc();

	setes(0x1000);

	for (i=1000; i<1000+64; i++){
		getblk(i, 0); putc('.');
		inces();
	}
	prints("\n\rready to go?"); getc();
}
