#include "ucode.c"

char userinput[128];
char templine[128];
char *usertoks[20];
char *commands[] = {
	"?",
	"help",
	"pwd",
	"cd",
	"exit",
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
	0
};
char *menu = "\
################################################\n\r\
# ls    cd     pwd    cat    cp    mv    ps    #\n\r\
# mkdir rmdir  creat  rm     chmod more  grep  #\n\r\
# exit  lpr                                    #\n\r\
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
		if (strcmp(cp, cmd) == 0)
			return i;
		i++;
		cp = commands[i];
	}
	return -1;
}


int main(int argc, char *argv[])
{
	int cmd, cpid, status, i;
	char *cp;
	char execline[128];
	char s[64];
	while(1){
		printf("bhsh # : ");
		gets(userinput);
		if(userinput[0]==0){
			continue;
		}
		strcpy(templine, userinput);
		linetoken(templine);
		cmd = getcmd(usertoks[0]);
		printf("cmd: %d %s\n", cmd, usertoks[0]);
		switch(cmd){
			case 0:
			case 1:
				printf(menu);
				continue;
			case 3:
				printf("dir: %s", usertoks[i+1]);
				chdir(usertoks[i+1]);
				continue;
			case 2:
				getcwd(s);
				printf("%s\n",s);
				continue;
			case 4:
				exit(0);
				break;
			default:
				break;;

		}
		cpid = fork();
		if (cpid == 0){ // child
			cp = execline;
			for (i = 0; usertoks[i] != 0; i++) {
				switch(usertoks[i][0]){
					case '>':
						break;
					case '<':
						break;
					case '|':
						break;
					default:
						strcpy(cp, usertoks[i]);
						cp += strlen(usertoks[i]);
						*(cp++) = ' ';
				}
			}
			*(cp-1) = 0;
			exec(execline);
			exit(1);
		} else { // parent
			cpid = wait(&status);
		}
	}
	return 0;
}
