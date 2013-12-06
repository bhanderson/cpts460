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

int iosetup(){
	int fd;
	int i;
	while(tokens[i]){
		printf("in while\n");
		switch(tokens[i][0]){
			case '>':
				write(1, "IN IO>\n", 7);
				close(1); // close stdout so open uses it
				if(tokens[i][1]=='>'){ // append to file
					fd = open(tokens[i+1], O_WRONLY | O_APPEND | O_CREAT);
					if(fd != 1)
						write(1, "APPEND ERROR\n", 13);
				} else {
					fd = open(tokens[i+1], O_WRONLY | O_CREAT);
					if(fd != 1)
						write(1, "WRITE ERROR\n", 12);
				}
				return i;
			case '<':
				return i;
			case '|':
				return i;
			default:
				i++;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	while(1){
		char inputline[128], temp_line[128], *tok;
		int i = 0, cmd, status, pid, fd, pos = 0;
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
					if(fd != 1)
						write(1, "WRITE ERROR\n", 12);
				}
				i++;
				/*
				switch(tokens[i][0]){
					case '>':
						write(1, "IN IO>\n", 7);
						close(1); // close stdout so open uses it
						if(tokens[i][1]=='>'){ // append to file
							fd = open(tokens[i+1], O_WRONLY | O_APPEND | O_CREAT);
							if(fd != 1)
								write(1, "APPEND ERROR\n", 13);
						} else {
							fd = open(tokens[i+1], O_WRONLY | O_CREAT);
							if(fd != 1)
								write(1, "WRITE ERROR\n", 12);
						}
						pos = i;
					case '<':
						return i;
					case '|':
						return i;
					default:
						i++;
				} // end switch*/
			} // end while
			printf("IO: %d char %s\n", pos, tokens[pos]);
			if(pos){// there is a io redirect
				write(1, "IO!\n", 4);
				exec(tokens[0]);
				exit(1);
			} else {
				exec(inputline);
				exit(1);
			}
		}
	}
	return 0;
}
