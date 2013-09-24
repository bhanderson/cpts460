#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
	"Saturn", "Uranus", "Neptune" };
// hilarious kcw expressions
char *gasp[NPROC]={
	"Oh! You are killing me .......",
	"Oh! I am dying ...............",
	"Oh! I am a goner .............",
	"Bye! Bye! World...............",
};

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
 **************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "wait.c"
#include "kernel.c"
#include "int.c"
// add a proc to a queue ordered by priority
void enqueue(PROC *p, PROC** queue){
	PROC *c, *n;
	// if the queue is empty or the new process has highest priority in queue
	if (queue == NULL){
		*queue = p;
		p->next = NULL;
		return;
	}
	if (p->priority > (*queue)->priority){
		p->next = *queue;
		*queue = p;
		return;
	}

	// go through the queue until the priority is <= the next
	c = (*queue);
	n = c->next;

	while(c != NULL && p->priority <= n->priority){
		c = n;
		n = n->next;
	}
	// then insert
	c->next = p;
	p->next = n;
	return;
}
// get the top process
PROC *dequeue(PROC **queue){
	PROC *p = *queue;
	printf("dequeued proc %d\n", p->pid);
	if (*queue != NULL)
		*queue = (*queue)->next;
	return p;
}
// return a the top fre process or return NULL
PROC* getproc(){
	if (freeList != NULL)
		return dequeue(&freeList);
	return freeList;
}
// print out a queue that is specified
void printQueue(PROC *queue){
	PROC *p = queue;
	while(p != NULL){
		printf("[%d] -> ", p->pid);
		p = p->next;
	}
	printf("NULL\n");
	printf("\n");
}
// setup the stacks of 9 processes to be populated
int kfork(){
	PROC *p = getproc();
	int i, segment;
	u16 word;

	if (p==NULL){
		printf("ERROR: no free procs\n");
		return -1;
	}
	p->ppid = running->pid;
	p->status = READY;
	p->parent = running;
	p->priority = 1;
	for (i=1; i<10; i++){
		p->kstack[SSIZE - i] = 0;          // all saved registers = 0
		p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
		p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
	}

	enqueue(p, &readyQueue);

	segment = (p->pid +1) * 0x1000;

	load("bin/u1", segment);

	printf("seg address: %d\n", segment);

	// initialize procs umode stack as if it was running in umode before, and
	// called syscall with no parameter from its virtual address 0 so when it
	// goumode() it would return to seg 0 to execute which is the betginning of
	// the loaded u1 code
	for (i = 0; i < 13; i++) {
		switch(i){
			case 1: word = 0x0200; break;
			case 2:
			case 11:
			case 12: word = segment; break;
			default: word = 0; break;
		}
		put_word(word, segment, 0x1000-i*2);
	}
	p->uss = segment;
	p->usp = 0x1000 -12*2;

	printf("Proc %d forked %d at %x\n", running->pid, p->pid, segment);

	return p->pid;
}
int init()
{
	PROC *p; int i;
	printf("init ....");
	for (i=0; i<NPROC; i++){   // initialize all procs
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		strcpy(proc[i].name, pname[i]);

		p->next = &proc[i+1];
	}
	freeList = &proc[0];      // all procs are in freeList
	proc[NPROC-1].next = 0;
	readyQueue = sleepList = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList);
	p->status = RUNNING;
	p->ppid   = 0;
	p->parent = p;
	running = p;
	nproc = 1;
	printf("done\n");
}

int scheduler()
{
	if (running->status == RUNNING){
		running->status = READY;
		enqueue(running, &readyQueue);
	}
	running = dequeue(&readyQueue);
	running->status = RUNNING;
}

int int80h();

int set_vec(vector, handler) u16 vector, handler;
{
	// put_word(word, segment, offset) in mtxlib
	put_word(handler, 0, vector<<2);
	put_word(0x1000,  0, (vector<<2) + 2);
}

main()
{
	printf("MTX starts in main()\n");
	init();      // initialize and create P0 as running
	set_vec(80, int80h);

	kfork("/bin/u1");     // P0 kfork() P1

	while(1){
		printf("P0 running\n");
		if (nproc==2 && proc[1].status != READY)
			printf("no runable process, system halts\n");
		while(!readyQueue);
		printf("P0 switch process\n");
		tswitch();   // P0 switch to run P1
	}
}
