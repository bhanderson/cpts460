#include "ucode.c"
char *commands[] = {
	"<",
	">>",
	">",
	"|",
	"?",
	"cat",
	"cd",
	"chmod",
	"cp",
	"creat",
	"exit",
	"grep",
	"help",
	"lpr",
	"ls",
	"mkdir",
	"more",
	"mv",
	"ps",
	"pwd",
	"rm",
	"rmdir",
	// shell commands
	0
};
char *tokens[32];

int getcmd(char *cmd){
	int i = 0;
	char *cp = commands[0];
	while(cp){
		if(strcmp(cp, cmd) == 0){
			return i;
		}
		i++;
		cp = commands[i];
	}
	return -1;
}

int main(int argc, char *argv[])
{
	while(1){
		char inputline[128], temp_line[128], *tok, execline[128];
		int i = 0, j, cmd, status, pid, fd, pos = 0;
		printf("bhsh # : ");
		gets(inputline);
		strcpy(temp_line, inputline);
		tok = strtok(temp_line, " ");
		while(tok){
			tokens[i]  = tok;
			tok = strtok(NULL, " ");
			i++;
		}
		tokens[i] = 0; // end tokens with 0

		cmd = getcmd(tokens[0]);
		printf("cmd: %d\n", cmd);
		pid = fork();
		if (pid){ // parent waits
			printf("in parent sh waiting\n");
			pid = wait(&status);
		} else { // child execs
			i = 0;
			while(tokens[i]){
				if(!strcmp(tokens[i], ">")){
					close(1);
					pos = i;
					fd = open(tokens[i+1], O_WRONLY | O_CREAT);
					if(fd != 1){
						write(1, "WRITE ERROR\n", 12);
						exit(-1);
					}
				}
				if(!strcmp(tokens[i], ">>")){
					close(1);
					pos = i;
					fd = open(tokens[i+1], O_WRONLY | O_CREAT | O_APPEND);
					if (fd != 1) {
						write(1, "WRITE ERROR\n", 12);
						exit(-1);
					}
				}
				if(!strcmp(tokens[i], "<")){
					close(0);
					pos = i;
					fd = open(tokens[i+1], O_RDONLY);
					if (fd != 0){
						write(1, "READ ERROR\n", 11);
						exit(-1);
					}
				}
				i++;
			}
			if(pos){// there is a io redirect
				for (j = 0; j < pos; j++) {
					strcat(execline, " ");
					strcat(execline, tokens[j]);
				}
				exec(execline);
				exit(1);
			} else {
				exec(inputline);
				exit(1);
			}
		}
	}
	return 0;
}
