#include "ucode.c"

void ppage(int morefd){
	int i, j, size;
	char c;
	/*
	   while(read(morefd, &c, 1) > 0){
	   putc(c);
	   }
	   return;
	   */
	putc('\n');
	for (i = 0; i < 24; i++) {
		for (j = 0; j < 80; j++) {
			size = read(morefd, &c, 1);
			putc(c);
			if (size < 1){
				exit(0);
			}
			if(c=='\n' || c=='\r')
				break;
		}
	}
	return;
}

void pline(int morefd){
	int j, size;
	char c;
	for (j = 0; j < 80; j++) {
		size = read(morefd, &c, 1);
		putc(c);
		if (size < 1){
			exit(0);
		}
		if (c == '\n' || c=='\r')
			break;
	}
	return;
}

int main(int argc, char *argv[])
{
	char c, input;
	int morefd, read, i, j;
	morefd = open(argv[1], O_RDONLY);
	printf("Bryce's more fd=%d\n\n", morefd);
	if (morefd<0) {
		printf("More cant find file\n");
		return -1;
	}
	while(1){
		ppage(morefd);
		printf("enter: next line q: quit space: next page");
		while (1) {
			input = getc();
			switch(input){
				case '\r':
					pline(morefd);
					break;
				case 'q':
					putc('\n');
					return 0;
					break;
				case ' ':
					ppage(morefd);
					break;
				default:
					ppage(morefd);
					break;
			}
		}
	}

	return 0;
}