#include "ucode.c"
int main(int argc, char *argv[])
{
	int sfd, dfd;
	int r;
	char buf[128];
	if (argc<3) {
		printf("Usage: grep source destination\n");
	}
	// open source
	sfd = open(argv[1], O_RDONLY);
	// open destination with create flag
	dfd = open(argv[2], O_WRONLY | O_CREAT);
	// if either one doesnt oopen
	if(sfd <0 || dfd < 0){
		printf("Cp cant open source or cant create destination file\n");
		return 1;
	}
	// write at least 128 
	while((r = read(sfd, buf, 128))>0){
		write(dfd, buf, r);
	}
	close(sfd);
	close(dfd);
	return 0;
}
