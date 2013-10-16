#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter",
	"Saturn", "Uranus", "Neptune" };

OFT  oft[NOFT];
PIPE pipe[NPIPE];

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
 **************************************************/
/******** YOUR .c files up to fork-exec ***********
#include "kernel.c"
 *************************************************/
#include "wait.c"
#include "int.c"
#include "forkexec.c"
#include "pipe.c"   // YOUR pipe.c file

int init()
{
	PROC *p;
	int i, j;
	printf("init ....");
	for (i=0; i<NPROC; i++){   // initialize all procs
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		strcpy(proc[i].name, pname[i]);
		p->next = &proc[i+1];

		for (j=0; j<NFD; j++)
			p->fd[j] = 0;
	}
	freeList = &proc[0];      // all procs are in freeList
	proc[NPROC-1].next = 0;
	readyQueue = sleepList = 0;

	for (i=0; i<NOFT; i++)
		oft[i].refCount = 0;
	for (i=0; i<NPIPE; i++)
		pipe[i].busy = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList);
	p->status = RUNNING;
	p->ppid   = 0;
	p->parent = p;
	running = p;
	nproc = 1;
	printf("done\n");
}

// execute switch and print what proc is switching
int do_tswitch(){
	printf("PID %d switching", running->pid);
	tswitch();
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
	p->priority = 1;                 // all of the same priority 1
	// clear kstack
	for (i = 1; i < 10; i++) {
		p->kstack[SSIZE -i] = 0;
	}
	p->kstack[SSIZE -1] =(int)body;
	p->ksp = &(p->kstack[SSIZE-9]);
	nproc++;
	// make Umode image by loading /bin/u1 into segment
	segment = (p->pid + 1)*0x1000;
	load(filename, segment);
	printf("loaded %s at %u\n", filename, segment);
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
	enqueue(&readyQueue, p);
	return(p->pid);
}
// execute kfork and check return
int do_kfork(){
	int i;
	i = kfork("bin/u1");
	if (i < 0)
		printf("fork failed\n");
	else
		printf("parent return from fork, child=%d\n", i);
}

// execute wait and check if success
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

int scheduler()
{
	if (running->status == RUNNING){
		running->status = READY;
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	running->status = RUNNING;
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

// print out all the processes
int do_ps(){
	int i;
	printf("==============================================\n");
	printf("\tname\t\t\tstatus\t\tpid\t\tppid\n");
	printf("----------------------------------------------\n");
	for (i = 0; i < NPROC; i++) {
		printf("\t%s\t\t\t\t\t",proc[i].name);
		switch(proc[i].status){
			case FREE:		printf("FREE     "); break;
			case READY:		printf("READY    "); break;
			case RUNNING:	printf("RUNNING  "); break;
			case STOPPED:	printf("STOPPED  "); break;
			case SLEEP:		printf("SLEEP    "); break;
			case ZOMBIE:	printf("ZOMBIE   "); break;
		}
		printf("%d\t\t\t",proc[i].pid);
		printf("%d\n",proc[i].ppid);
	}
	printf("----------------------------------------------\n");
	return 0;
}

int kmode(){
	body();
}

// change the name of a process
int chname(char *c){
	int i;
	// get the name from umode
	for (i = 0; i < 32; i++) {
		running->name[i] = get_byte(running->uss, c+i);
		if (running->name[i] == '\0')
			break;
		else
			printf("%c", running->name[i]);
	}
	printf("\n");
	running->name[31] = '\0';
}

int int80h();

int set_vec(vector, addr) u16 vector, addr;
{
	u16 location,cs;
	location = vector << 2;
	put_word(addr, 0, location);
	put_word(0x1000,0,location+2);
}



main()
{
	printf("MTX starts in main()\n");
	init();      // initialize and create P0 as running
	set_vec(80,int80h);

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
