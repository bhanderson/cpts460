#include "ucode.c"
void clrline(char *line){
	int i;
	for (i = 0; i < 160; i++) {
		line[i]=0;
	}
}
int main(int argc, char *argv[])
{
	int grepfd, size, i, linenum = 0;
	char line[160];
	char *c = line;
	int data_read = 1;
	clrline(&line);
	if (argc < 3) {
		printf("Usage: grep PATTERN [FILE]\n");
		return 0;
	}
	grepfd = open(argv[2], O_RDONLY);
	if (grepfd<0) {
		printf("Grep cant find file\n");
		return 1;
	}
	size = strlen(argv[1]);
	while (data_read > 0){
		linenum++;
		for (i = 0; i < 159; i++) {
			data_read = read(grepfd, &line[i], 1);
			if (data_read < 1 || line[i] == '\n') {
				line[i+1] = 0;
				break;
			}
		}
		c = line;
		while (*c != '\n'){
			if (strncmp(c, argv[1], size) == 0 && line[0] != 0){
				printf("%d: %s\r",linenum, line);
				break;
			}
			c++;
		}
		clrline(&line);
	}
	close(grepfd);
	return 0;
}
