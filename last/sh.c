#include "ucode.c"
int main(int argc, char *argv[])
{
	while(1){
		char inputline[128], *tokens[32], *tok;
		int i = 0;
		printf("bhsh # : ");
		gets(inputline);
		printf("command: %s\n",inputline);
		tok = strtok(inputline, " ");
		while(tok){
			tokens[i++] = tok;
			tok = strtok(NULL, " ");
		}
	}
	return 0;
}
