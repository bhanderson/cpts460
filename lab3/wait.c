#include "type.h"
int ksleep(event) int event;
{
	running->event = event;
	running->status = SLEEP;
	enqueue(running, &sleepList);

	tswitch();
}

/* wake up ALL procs sleeping on event */
int kwakeup(event) int event;
{
	PROC *past = sleepList, *curr = sleepList;
	int i = 0;
	if (past == NULL)
		return -1;

	while (curr != NULL) {
		if (curr->event == event) {
			if (curr == sleepList){
				sleepList = curr->next;
				curr->event=0;
				curr->status = READY;
				enqueue(p, &readyQueue);
				printf("woke %d\n", curr->pid);
				p = q = sleepList;
				continue;
			}
			// if not the first item in the list
			past->next = curr->next;
			curr->event = 0;
			curr->status = READY;
			enqueue(p, &readyQueue);
			printf("woke %d\n", curr->pid);
			curr = past->next;
		}
		past = curr;
		curr = past->next;
	}
	return 0;
}

int kexit(exitValue) int exitValue;
{
	int i;
	PROC *p;
	printf("%d exit %d\n", running->pid, exitValue);
	if (running->pid == proc[1].pid) {
		for (i = 2; i < NPROC; i++) {
			if (proc[i].status != FREE && proc[i].ppid == proc[1].pid){
				printf("proc %d not free\n", i);
				return -1;
			}
		}
	}
	running->status = ZOMBIE;
	running->exitValue = exitValue;
	printf("*****************************************\n");
	printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
	printf("*****************************************\n");
	for (i = 0; i < NPROC; i++) {
		if (proc[i].ppid == running->pid){
			proc[i].ppid = proc[1].pid;
		}
	}
	kwakeup(&proc[running->ppid]);

	tswitch();
}

int kwait(status) int *status;
{
	int i, count, pid;
	for (i = 0, count = 0; i < NPROC; i++) {
		if(proc[i].ppid == running->pid)
			count++;
	}
	if (count == 0){
		printf("no children to wait on\n");
		return -1;
	}while(1){
		for (i = 0; i < NPROC; i++) {
			if (proc[i].ppid == running->pid && proc[i].status == ZOMBIE) {
				*status = proc[i].exitVal;
				proc[i].status = FREE;
				return proc[i].pid;
			}
		}
		ksleep(running);
	}

}
