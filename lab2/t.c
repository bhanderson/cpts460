/************ t.c file **********************************/
#define NPROC		9
#define SSIZE		1024           /* kstack int size */

#define DEAD		0              /* proc status     */
#define READY		1
#define FREE		2
#define NULL		0

typedef struct proc{
	struct proc *next;
	int  ksp;               /* saved sp; offset = 2 */
	int  pid;
	int  ppid;
	int  priority;
	int  status;            /* READY|DEAD, etc */
	int  kstack[SSIZE];     // kmode stack of task
}PROC;

//#include "io.c"  /* <===== use YOUR OWN io.c with printf() ****/

PROC proc[NPROC], *running, *readyQueue, *freeList;

int  procSize = sizeof(PROC);

/****************************************************************
  Initialize the proc's as shown:
  running ---> proc[0] -> proc[1];

  proc[1] to proc[N-1] form a circular list:

  proc[1] --> proc[2] ... --> proc[NPROC-1] -->
  ^                                         |
  |<---------------------------------------<-

  Each proc's kstack contains:
  retPC, ax, bx, cx, dx, bp, si, di, flag;  all 2 bytes
 *****************************************************************/

int body();
int kfork();

int initialize(){
	int i, j;
	PROC *p;

	for (i=0; i < NPROC; i++){
		p = &proc[i];
		p->next = &proc[i+1];
		p->pid = i;
		p->priority = 1;
		p->status = FREE;
	}
	running = &proc[0];
	running->priority = 0;
	running->status = READY;
	running->next = NULL;
	freeList = &proc[1];
	proc[NPROC-1].next = &proc[1];
	readyQueue = NULL;
	printf("initialization complete\n");
}

char *gasp[NPROC]={
	"Oh! You are killing me .......",
	"Oh! I am dying ...............",
	"Oh! I am a goner .............",
	"Bye! Bye! World...............",
};

int grave(){
	printf("\n*****************************************\n");
	printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
	printf("*****************************************\n");
	running->status = DEAD;

	tswitch();   /* journey of no return */
}

int ps(){
	PROC *p;

	printf("running = %d\n", running->pid);

	p = running;
	p = p->next;
	printf("readyProcs = ");
	while(p != running && p->status==READY){
		printf("%d -> ", p->pid);
		p = p->next;
	}
	printf("\n");
}

int body(){
	char c;
	while(1){
		ps();
		printf("I am Proc %d in body()\n", running->pid);
		printf("Input a char : [f|s|q] ");
		c=getc();
		switch(c){
			case 'f': kfork();		break;
			case 's': tswitch();	break;
			case 'q': grave();		break;
			default :				break;
		}
	}
}

main(){
	printf("\nWelcome to the 460 Multitasking System\n");
	initialize();
	kfork();
	printf("P0 switch to P1\n");
	tswitch();
	printf("P0 resumes: all dead, happy ending\n");
}

void enqueue(PROC *p, PROC** queue){
	//printf("added process: %d to queue \n", p->pid);
	PROC *c, *n;
	// if the queue is empty or the new process has highest priority in queue
	if (queue == NULL || p->priority > (*queue)->priority){
		PROC* temp = *queue;
		*queue = p;
		p->next = temp;
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

PROC *dequeue(PROC **queue){
	PROC *p = *queue;
	if (*queue != NULL)
		*queue = (*queue)->next;
	return p;
}

void printQueue(PROC *queue){
	PROC *p = queue;
	while(p != NULL){
		printf("[%d] -> ", p->pid);
		p = p->next;
	}
	printf("NULL\n");
}

int scheduler(){
	if (running->status == READY)
		enqueue(running, &readyQueue);

	running = dequeue(&readyQueue);
	printf("\n-----------------------------\n");
	printf("next running proc = %d\n", running->pid);
	printf("-----------------------------\n");
}

PROC* getproc(){
	if (freeList != NULL)
		return dequeue(&freeList);
	return freeList;
}

int kfork(){
	PROC *p = getproc();
	int i;

	if (p==NULL){
		printf("ERROR: no free procs\n");
		return -1;
	}

	for (i=1; i<10; i++)
		p->kstack[SSIZE - i] = 0;          // all saved registers = 0
	p->ppid = running->pid;
	p->status = READY;
	p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
	p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top

	enqueue(p, &readyQueue);
	return p->pid;
}
