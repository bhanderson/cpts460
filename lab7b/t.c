#include "type.h"
PROC* getproc();
PROC* get_proc(PROC * list);
void enqueue(PROC **queue, PROC *p);
PROC *dequeue(PROC **queue);
PROC proc[NPROC], *freeList, *sleepList, *running, *readyQueue;
int procSize = sizeof(PROC);
int goUmode();
/******************** use your OWN LAB5 code *****************
#include "io.c"
#include "queue.c"
#include "wait.c"
#include "fork_exec.c"
 *************************************************************/

#include "int.c"
#include "pv.c"
#include "serial.c"
#include "kernel.h"

int body();

char *tname[]={"Mercury", "Venus", "Earth",  "Mars", "Jupiter",
	"Saturn",  "Uranus","Neptune"};
int init()
{
	int i,j;
	PROC *p;
	for (i=0; i<NPROC; i++){
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->next = &proc[i+1];
		strcpy(p->name, tname[i]);
		printf("%d %s ", p->pid, proc[i % 4].name);
	}
	printf("\n");

	proc[NPROC-1].next = 0;
	freeList = &proc[0];
	readyQueue = 0;

	// create and run P0
	p = getproc();
	p->priority = 0;
	p->status = READY;
	p->inkmode = 1;
	running = p;
}

int body()
{
	char c, CR, buf[64];
	while(1){
		printf("=======================================\n");
		printQueue(readyQueue);

		printf("proc %d %s in Kmode\n", running->pid, running->name);
		printf("input a command (s|f|u|q|i|o) : ");
		c=getc(); putc(c); CR=getc(); putc(CR);
		switch(c){
			case 's' : tswitch(); break;
			case 'u' : printf("\nProc %d ready to go U mode\n", running->pid);
					   goUmode(); break;
			case 'f':  myfork();  break;
			case 'q' : kexit();   break;
			case 'i' : iline();   break;
			case 'o' : oline();   break;
		}
	}
}

/***************************************************************
  myfork(segment) creates a child task and returns the child pid.
  When scheduled to run, the child task resumes to body(pid) in
  K mode. Its U mode environment is set to segment.
 ****************************************************************/
int myfork()
{
	int child;
	child = kfork("/bin/u1");
	if (child < 0){
		printf("myfork failed\n");
		return;
	}
	printf("task %d return from myfork() : child=%d\n", running->pid, child);
}

int int80h(), s0inth();  // int s1inth() // for serial port 1

int set_vec(vector, addr) ushort vector, addr;
{
	ushort location,cs;
	location = vector << 2;
	cs = getcs();
	put_word(addr, 0, location);
	put_word(cs,0,location+2);
}

main()
{
	int pid;
	resetVideo();
	printf("\nWelcome to the 460 Multitasking System\n");
	printf("initializing ......\n");
	init();
	printf("initialization complete\n");

	set_vec(80, int80h);

	pid = myfork("/bin/u1");

	set_vec(12, s0inth);
	//set_vec(11, s1inth); // for second serial port at 0x2F8
	sinit();

	while(1){
		if (readyQueue)
			tswitch();
	}
	printf("all dead, happy ending\n");
}

int scheduler()
{
	if (running->status == READY)
		enqueue(running, &readyQueue);
	running = dequeue(&readyQueue);
}


int oline()
{
	// assume port = 0;
	sputline(0, "serial line from Kmode\n\r");
}

char kline[64];
int iline()
{
	sgetline(0, kline);
	printf("\nkline=%s\n", kline);
}
