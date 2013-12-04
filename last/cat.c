#include "ucode.c"

int main(int argc, char *argv[])
{
	char c;
	int catfd;
	catfd = open(argv[1], O_RDONLY);
	printf("Bryce's almost cool cat MEOW fd=%d\n\n", catfd);
	if(catfd<0){
		printf("Cat cant meow file\n");
		return -1;
	}
	while (read(catfd, &c, 1)>0) {
		if (c == '\n'){
			putc('\n');
			putc('\r');
		}else{
			putc(c);
		}
	}
	printf("\n");
	close(catfd);
	printf("fd: %d closed\n", catfd);
	return 0;
}


