/*
 * 5-2. Write YOUR OWN sh.c to support I/O redirections and pipes:

Examples:  cat [filename]   // NOTE: cat may NOT need a filenmae
cat [filename] >  newfile
cat [filename] >> appendFile
a.out < inFile    read inputs from inFile
cat filename | more
cat filename | grep test
cat filename | grep print | more

cat filename > /dev/lp0    print filename
cp  filename /dev/lp0      print filename
*/

#include "ucode.c"
char *commands[] = {
	"?",
	"cd",
	"exit",
	"help",
	"pwd",
	"<",
	">>",
	">",
	"|",
	"cat",
	"chmod",
	"cp",
	"creat",
	"grep",
	"lpr",
	"ls",
	"mkdir",
	"more",
	"mv",
	"ps",
	"rm",
	"rmdir",
	// shell commands
	0
};
char cmdline[128];
char *usertoks[20];
char tokline[128];
char *help = "\
################################################\n\r\
# ls    cd     pwd    cat    cp    mv    ps    #\n\r\
# mkdir rmdir  creat  rm     chmod more  grep  #\n\r\
# exit  help   lpr                             #\n\r\
# (I/O and Pipe) :    >      >>    <     |     #\n\r\
################################################\n\r";

	 void linetoken(char *line){
		 int i = 0;
		 char *token;
		 token = strtok(line, " ");
		 while(token!= NULL){
			 usertoks[i] = token;
			 token = strtok(NULL, " ");
			 i++;
		 }
	 }
int getcmd(char *cmd){
	int i;
	char * cp = commands[0];
	while(cp){
		//printf("%s", cmd);
		if (strcmp(cp, cmd) == 0)
			return i;
		i++;
		cp = commands[i];
	}
	return -1;
}
void shcmd(int c){
	char s[32];
	switch(c){
		case 0:
		case 3:
			printf("%s", help);
			break;
		case 1:
			//printf("%s",(char *)usertoks[1]);
			chdir(usertoks[1]);
			break;
		case 2:
			exit(0);
			break;
		case 4:
			getcwd(s);
			printf("%s\n", s);
			break;
		default:
			printf("shcmd error\n");
			break;
	}

}

void myrun(int c){
	int mypipe[2];
	char newline[128];
	int cpid, i;
	char *cp = newline;
	int tempint;
	STAT temp_stat;
	for (i = 0; usertoks[i] != 0; i++) {
		// specific for io ops
		switch(usertoks[i][0]){
			case '>':
				close(1);
				if(usertoks[i][1] == '>'){ // append
					i++;
					if (open(usertoks[i], O_WRONLY | O_CREAT | O_APPEND) != 1){
						write(2, "Cannot open file\n", 17);
						return;
					}
				} else { // create
					i++;
					if (open(usertoks[i], O_WRONLY | O_CREAT) != 1){
						write(2, "Cannot create file\n", 21);
						return;
					}
				}
				break;
			case '<':
				close(0);
				i++;
				//printf("io: %s", usertoks[i]);
				open(usertoks[i], O_RDONLY);
				//printf("fd: %d\n", tempint);
				//i++; // get rid of 
				//printf("tok: %s", usertoks[i]);
				break;
			case '|':
				pipe(mypipe);
				cpid = fork();
				if(cpid == -1){
					printf("fork error");
					//exit(-1);
					return;
				}
				break;

			default:
				strcpy(cp, usertoks[i]);
				printf("cp: %s", cp);
				cp += strlen(usertoks[i]);
				*(cp++) = ' ';
				break;
		}
	}
	*(cp-1) = 0;
	write(2, newline, strlen(newline));
	exec(newline);
	return;
}

int main(int argc, char *argv[])
{
	int cmd;
	while(1){// main program loop
		printf("bhsh # : ");
		gets(cmdline);
		if (cmdline[0]==0) {
			continue;
		}
		strcpy(tokline, cmdline);
		// tokenize the input
		linetoken(tokline);

		// find the first command of the process
		//printf("cmd:%s\n",usertoks[0]);
		if ((cmd = getcmd(usertoks[0])) < 5){
			shcmd(cmd);
		} else {
			myrun(cmd);
		}
	}
	return 0;
}
