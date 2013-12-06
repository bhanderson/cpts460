#include "ucode.c"

int main(int argc, char *argv[])
{
	char c;
	int catfd;
	if (argc > 1){
		catfd = open(argv[1], O_RDONLY);
	} else {
		catfd = 0;
	}
	printf("Bryce's almost cool cat MEOW fd=%d\n\n", catfd);
	if(catfd<0){
		printf("Cat cant meow file\n");
		return -1;
	}
	while (read(catfd, &c, 1)>0) {
		putc(c);
		if (catfd != 0 && c == '\n')
			putc('\r');
		if (catfd == 0 && c == '\r'){
			putc('\n');
			putc('\r');
		}
	}
	printf("\n");
	close(catfd);
	printf("fd: %d closed\n", catfd);
	return 0;
}


