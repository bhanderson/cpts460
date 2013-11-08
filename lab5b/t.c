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
 *************************************************/

//#include "kernel.c"
#include "forkexec.c"
#include "int.c"
#include "wait.c"
#include "pipe.c"   // YOUR pipe.c file

int init()
{
	PROC *p;
	int i, j;
	printf("init ....");
	for (i=0; i<NPROC; i++){   // initialize all procs
		p = &proc[i];
		p->pid = i;
		p->ppid = 0;
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

int scheduler()
{
	if (running->status == RUNNING){
		running->status = READY;
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	running->status = RUNNING;
}

int int80h();

int set_vec(vector, addr) u16 vector, addr;
{
	u16 location,cs;
	location = vector << 2;
	put_word(addr, 0, location);
	put_word(0x1000,0,location+2);
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
	segment = (p->pid + 1)*0x2000;
	load("/bin/u1", segment);
	printf("loaded %s at %u\n", "/bin/u1", segment);
	for (i = 1; i < 13; i++) {
		switch(i){
			case 1:		child = 0x0200;		break;
			case 2:
			case 11:
			case 12:	child = segment;	break;
			default:	child = 0;			break;
		}
		put_word(child, segment, 0x2000-i*2);
	}
	p->uss = segment;
	p->usp = 0x2000 - 12*2;

	printf("Proc%d forked a child %d segment=%x\n", running->pid,p->pid,segment);
	enqueue(&readyQueue, p);
	return(p->pid);
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
