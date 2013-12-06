int pid, child, status;
int stdin, stdout, stderr;

#include "ucode.c"
// ttyS0 ttyS1

int main(int argc, char *argv[])
{
	stdin = open("/dev/tty0", 0);
	stdout = open("/dev/tty0", 1);
	stderr = open("/dev/tty0", 2);

	printf("BHINIT : fork a login task on console\n");
	child = fork();
	if (child)
		parent();
	else
		login();
	return 0;
}
int login(char *s)
{
	exec(s);
}
int parent()
{
	while (1) {
		printf("BHINIT : waiting .....\n");
		pid = wait(&status);
		if (pid == child){
			printf("BHINIT : forking another login\n");
			child = fork();
			if (!child) {
				login("login /dev/tty0");
				login("login /dev/ttyS0");
				login("login /dev/ttyS1");
			}
		} else
			printf("INIT : buried an orphan child %d\n", pid);

	}
}
