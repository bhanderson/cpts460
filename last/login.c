#ifndef NULL
#define NULL 0
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#include "ucode.c"

char *tty;
int stdin, stdout, stderr;

int main(int argc, char *argv[]){
	char username[32], user[32], password[32], pass[32], buf[1024];
	char name[32], program[32], home[32];
	int valid = false;
	int passwdfd, gid, uid;
	char *tok;

	tty = argv[1];

	close(0);
	close(1);
	close(2);

	stdin = open(tty, 0);
	stdout = open(tty, 1);
	stderr = open(tty, 2);

	fixtty(tty);

	printf("BHLOGIN : open %s as stdin, stdout, stderr\n", tty);

	signal(2,1);	// ignore Control-C interrupts so that Ctrl-C kills
					// other procs on this tty but not the main sh
	while (1) {
		// show login to stdout
		printf("login as: ");
		// read username from stdin
		gets(username);
		printf("\n");
		// show passwd:
		printf("%s@BHLOGIN's password: ", username);
		// read user passwd
		gets(password);
		// verify user name and passwd from /etc/passwd file
		passwdfd = open("/etc/passwd", O_RDONLY);
		if (read(passwdfd, buf, 1024)<0){
			printf("ERROR READING /etc/passwd");
		}
		tok = strtok(buf,":\n");
		while(tok != 0 && valid == false){
			if (strcmp(tok, username) == 0){
				strcpy(user, username);
				strcpy(pass, strtok(NULL, ":\n"));
				// if (user account valid)
				if (strcmp(pass, password) == 0){
					valid = true;
					// set uid to user uid.
					uid = atoi(strtok(NULL, ":\n"));
					// set gid to user gid
					gid = atoi(strtok(NULL, ":\n"));
					// get full name of user
					strcpy(name, strtok(NULL, ":\n"));
					// get user home directory
					strcpy(home, strtok(NULL, ":\n"));
					// get user shell or program to run
					strcpy(program, strtok(NULL, ":\n"));
					// chdir to user HOME dir
					chdir(home);
					// exec to the program in the user's account
					exec(program);
				} else {
					printf("pass: %s for %s is invalid\n", password, username);
					break;
				}
			}
			tok = strtok(NULL, ":\n");
		}
	}
	close(passwdfd);
	return 0;
}
