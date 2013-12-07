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

char *menu = "\
################################################\n\r\
# ls    cd     pwd    cat    cp    mv    ps    #\n\r\
# mkdir rmdir  creat  rm     chmod more  grep  #\n\r\
# exit  lpr                                    #\n\r\
# (I/O and Pipe) :    >      >>    <     |     #\n\r\
################################################\n\r";



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
		char *cp;
		int i = 0, j, cmd, status, pid, fd, pos = 0;
		int pipes[2];
		char cwd[64];
		printf("bhsh # : ");
		gets(inputline);
		strcpy(temp_line, inputline);
		// tokenize all the input
		tok = strtok(temp_line, " ");
		while(tok){
			tokens[i]  = tok;
			tok = strtok(NULL, " ");
			i++;
		}
		tokens[i] = 0; // end tokens with 0
		cmd = getcmd(tokens[0]);
		// check if its a basic command
		switch(cmd){
			case 4: // ?
				printf(menu);
				break;
			case 6: // cd
				if(tokens[1])
					chdir(tokens[1]);
				else
					chdir("/");
				break;
			case 10: // exit
				exit(0);
				exit(0);
				break;
			case 12: // help
				printf(menu);
				break;
			case 19: // pwd
				getcwd(cwd);
				printf("%s\n", cwd);
				break;
			default:
				break;
		}
		//printf("cmd: %d\n", cmd);
		// fork that child
		pid = fork();
		if (pid){ // parent waits
			//printf("in parent sh waiting\n");
			pid = wait(&status);
		} else { // child execs
			i = 0;
			cp = inputline;
			for(i=0;tokens[i];i++){
				// check if it has a io redirection
				switch(tokens[i][0]){
					case '>':
						close(1);
						if(tokens[i][1] == '>')
							fd = open(tokens[i+1], O_WRONLY | O_CREAT | O_APPEND);
						fd = open(tokens[i+1], O_WRONLY | O_CREAT);
						if(fd != 1){
							write(1, "WRITE ERROR\n", 12);
							exit(-1);
						}
						break;
					case '<':
						close(0);
						fd = open(tokens[i+1], O_RDONLY);
						if (fd != 0){
							write(1, "READ ERROR\n", 11);
							exit(-1);
						}
						break;
					case '|':
						// initialize the pipes
						pipe(pipes);
						// fork another child
						pid = fork();
						// if we are the child close writer and reader of parent
						if (pid){
							tokens[i+1] = '\0';
							close(pipes[0]);
							close(1);
							dup2(pipes[1], 1); // copy the pipe over
						} else {
							// else run the command and ouput to the child
							cp = inputline;
							close(pipes[1]);
							close(0);
							dup2(pipes[0], 0);
						}
						break;
					default:
						strcpy(cp, tokens[i]);
						cp+= strlen(tokens[i]);
						*(cp++) = ' ';
				} // end switch
			} // end for
			*(cp-1) = 0;
			//*(execline + tokens + i) = '\0';
			printf("line: %s\n", inputline);
			exec(inputline);
			exit(0);
		} // end else child
	} // end infinate while
	return 0;
}
