#ifndef O_RDONLY
#define O_RDONLY 1
#endif

char *tty;
int stdin, stdout, stderr;

int main(int argc, const char *argv[])
{
	char username[32], password[32];
	char *line = 0;
	size_t len = 0;
	ssize_t read;

	char *tok;
	FILE *passwd;

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
		passwd = open("/etc/passwd", O_RDONLY);
		while(1){
			
		}


		// if (user account valid)
		// 		setuid to user uid.
		// 		chdir to user HOME dir
		// 		exec to the program in the user's account
		// 	else
		// 		printf("login failed, try again\n");
	}
	return 0;
}
