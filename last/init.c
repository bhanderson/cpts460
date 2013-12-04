int pid, child, status;
int stdin, stdout, stderr;

#include "ucode.c"

int main(int argc, const char *argv[])
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
int login()
{
	exec("login /dev/tty0");
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
				login();
			}
		} else
			printf("INIT : buried an orphan child %d\n", pid);

	}
}
