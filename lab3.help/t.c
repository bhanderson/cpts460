#include "type.h"
#include "queue.c"
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"P0", "P1", "P2", "P3",  "P4", "P5","P6", "P7", "P8" };

#include "wait.c"
#include "int.c"

int do_tswitch(){
	printf("PID %d switching", running->pid);
	tswitch();
}

int do_kfork(){
	int i;
	i = kfork();
	if (i < 0)
		printf("fork failed\n");
	else
		printf("parent return from fork, child=%d\n", i);
}

int do_wait(int *ptr){
	int val, pid;

	pid = wait(&val);

	if (ptr == 0){
		*ptr = val;
	} else {
		put_word(val, running->uss, ptr);
	}
	printf("waiting");
	return pid;
}



int do_ps(){
	int i;
	printf("================================================\n");
	printf("\tname\t\tstatus\tpid\tppid\n");
	printf("------------------------------------------------\n");
	for (i = 0; i < NPROC; i++) {
		printf("\t%s\t\t",proc[i].name);
		switch(proc[i].status){
			case FREE:		printf("FREE\t"); break;
			case READY:		printf("READY\t"); break;
			case RUNNING:	printf("RUNNING\t"); break;
			case STOPPED:	printf("STOPPED\t"); break;
			case SLEEP:		printf("SLEEP\n"); break;
			case ZOMBIE:	printf("ZOMBIE\n"); break;
		}
		printf("%d\t",proc[i].pid);
		printf("%d\n",proc[i].ppid);
	}
	printf("------------------------------------------------\n");
	return 0;
}

int kmode(){
	body();
}

int chname(char *c){
	int i;
	printf("name:");
	for (i = 0; i < 32; i++) {
		running->name[i] = get_byte(running->uss, c[i]);
		if (running->name[i] == '\0')
			break;
		else
			printf("%c", running->name[i]);
	}
	printf("\n");
	running->name[31] = '\0';
}

int init(){
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
	p->status = READY;
	p->ppid   = 0;
	p->parent = p;
	running = p;
	nproc = 1;
	printf("done\n");
}

int int80h();

int set_vec(u16 vector, u16 handler){
	// put_word(word, segment, offset) in mtxlib
	put_word(handler, 0, vector<<2);
	put_word(0x1000,  0, (vector<<2) + 2);
}

main(){
	printf("MTX starts in main()\n");
	init();      // initialize and create P0 as running
	set_vec(80, int80h);

	kfork("/bin/u1");     // P0 kfork() P1

	while(1){
		printf("P0 running\n");
		printf("%d", readyQueue);
		while(!readyQueue);
		printf("P0 switch process\n");
		tswitch();         // P0 switch to run P1
		printf("\n1\n");
	}
}

int scheduler(){
	if (running->status == READY)
		enqueue(running, &readyQueue);
	running = dequeue(&readyQueue);
}

int body(){
	char c;
	printf("proc %d resumes to body()\n", running->pid);
	while(1){
		printf("-----------------------------------------\n");
		printList("freelist  ", freeList);
		printList("readyQueue", readyQueue);
		printList("sleepList ", sleepList);
		printf("-----------------------------------------\n");

		printf("proc %d running: parent = %d  enter a char [s|f|w|q|u] : ",
				running->pid, running->parent->pid);

		c = getc(); printf("%c\n", c);
		switch(c){
			case 's' : do_tswitch();   break;
			case 'f' : do_kfork();     break;
			case 'w' : do_wait(0);      break;
			case 'q' : do_exit(100);      break;
			case 'u' : goUmode();      break;
		}
	}
}

int kfork(char *filename){
	PROC *p;
	int  i, child;
	u16  segment;

	/*** get a PROC for child process: **/
	if ( (p = get_proc(&freeList)) == 0){
		printf("no more proc\n");
		return(-1);
	}

	/* initialize the new proc and its stack*/
	p->status = READY;
	p->ppid = running->pid;
	p->parent = running;
	p->priority  = 1;                 // all of the same priority 1

	for (i = 1; i < 10; i++) {
		p->kstack[SSIZE -i] = 0;
	}
	p->kstack[SSIZE -1] =(int)body;
	p->ksp = &(p->kstack[SSIZE-9]);
	enqueue(p, &readyQueue);

	// make Umode image by loading /bin/u1 into segment
	segment = (p->pid + 1)*0x1000;
	load(filename, segment);
	for (i = 1; i < 13; i++) {
		switch(i){
			case 1:		child = 0x0200;		break;
			case 2:
			case 11:
			case 12:	child = segment;	break;
			default:	child = 0;			break;
		}
		put_word(child, segment, 0x1000-i*2);
	}
	p->uss = segment;
	p->usp = 0x1000 - 12*2;

	printf("Proc%d forked a child %d segment=%x\n", running->pid,p->pid,segment);
	return(p->pid);
}


