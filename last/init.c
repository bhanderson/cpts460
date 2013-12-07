int pid, status;
int children[3] = 0;
int stdin, stdout, stderr;

#include "ucode.c"

int main(int argc, char *argv[])
{
	stdin = open("/dev/tty0", 0);
	stdout = open("/dev/tty0", 1);
	stderr = open("/dev/tty0", 2);

	printf("BHINIT : fork a login task on console\n");
	children[0] = fork();
	// check all children if dont exist open all the files else login that one 
	if (children[0]) {
		children[1] = fork();

		if (children[1]) {
			children[2] = fork();

			if (children[2]) {
				parent();

			} else {
				login(2);
			}

		} else {
			login(1);
		}

	} else {
		login(0);
	}

	return 0;
}
// execute teh login program
int login(int i)
{
	switch(i){
		case 0:
			exec("login /dev/tty0");
			break;
		case 1:
			exec("login /dev/ttyS0");
			break;
		case 2:
			exec("login /dev/ttyS1");
			break;
		default:
			break;
	}
}
// if the original child isnt there
int parent()
{
	while (1) {
		printf("BHINIT : waiting .....\n");
		pid = wait(&status);
		// check if i am a child
		if (pid == children[0] || pid == children[1] || pid == children[2]){
			printf("BHINIT : forking another login\n");
			if (pid == children[0]){
				children[0] = fork();
				if(!children[0])
					login(0);
			} else if (pid == children[1]){
				children[1] = fork();
				if(!children[1])
					login(1);
			} else if (pid == children[2]){
				children[2] = fork();
				if(!children[2])
					login(2);
			}
		} else {
			printf("INIT: burying child %d", pid);
		}
	}
}
