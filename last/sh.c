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
		int i = 0, j, cmd, status, pid, fd, pos = 0;
		int pipes[2];
		char cwd[64];
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
		if(cmd == 10){
			exit(0);
		}
		//printf("cmd: %d\n", cmd);
		pid = fork();
		if (pid){ // parent waits
			//printf("in parent sh waiting\n");
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
				if(!strcmp(tokens[i], "|")){
					pipe(pipes);
					pid = fork();
					if (pid){
						tokens[i+1] = '\0';
						close(pipes[0]);
						close(1);
						dup2(pipes[1], 1);
					} else {
						close(pipes[1]);
						close(0);
						dup2(pipes[0], 0);
					continue;
					}
				}
				i++;
			}
			if(pos){// there is an io redirect
				for (j = 0; j < pos; j++) {
					strcat(execline, " ");
					strcat(execline, tokens[j]);
				}
				exec(execline);
				exit(1);
			} else {
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
						exec(inputline);
						exit(1);
						break;
				}
			}
		}
	}
	return 0;
}
