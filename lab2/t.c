/************ t.c file **********************************/
#define NPROC		9
#define SSIZE		1024           /* kstack int size */

#define DEAD		0              /* proc status     */
#define READY		1
#define FREE		2
#define ZOMBIE		3
#define SLEEP		4
#define NULL		0

typedef struct proc{
	struct proc *next;
	int  ksp;               /* saved sp; offset = 2 */
	int  pid;
	int  ppid;
	int  priority;
	int  status;            /* READY|DEAD|FREE, etc */
	int  exitVal;
	int  event;
	int  kstack[SSIZE];     // kmode stack of task
}PROC;

//#include "io.c"  /* <===== use YOUR OWN io.c with printf() ****/

PROC proc[NPROC], *running = NULL, *readyQueue = NULL, *freeList = NULL, *sleepList= NULL;

int  procSize = sizeof(PROC);
int body();
int kfork();
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
// setup proc structs and set proc 0 as READY
int initialize(){
	int i, j;
	PROC *p;
	// initialize all procs as p>0
	for (i=0; i < NPROC; i++){
		p = &proc[i];
		p->next = &proc[i+1];
		p->pid = i;
		p->priority = 1;
		p->status = FREE;
	}
	running = &proc[0];
	// p[0] gets special stuff
	running->priority = 0;
	running->status = READY;
	freeList = &proc[1];
	proc[NPROC-1].next = NULL;
	printf("initialization complete\n");
}
// hilarious kcw expressions
char *gasp[NPROC]={
	"Oh! You are killing me .......",
	"Oh! I am dying ...............",
	"Oh! I am a goner .............",
	"Bye! Bye! World...............",
};
// wake up a process from sleeping using an event
void wakeup(int event){
	int i;
	PROC *p1 = sleepList;
	if (p1 == NULL)
		return;
	while (p1 != NULL) {
		if (p1->event == event) {
			p1->status = READY;
			p1 = dequeue(&p1);
			enqueue(p1, &readyQueue);
			return;
		} else
			p1 = p1->next;
	}
}
// kill a process
int kexit(int val)
{
	int i;
	if (running->pid == proc[1].pid) {
		for (i = 2; i < NPROC; i++) {
			if (proc[i].status != FREE && proc[i].ppid == proc[1].pid){
				printf("proc %d not free\n", i);
				return;
			}
		}
	}
	running->status = ZOMBIE;
	running->exitVal = val;
	printf("*****************************************\n");
	printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
	printf("*****************************************\n");
	for (i = 0; i < NPROC; i++) {
		if (proc[i].ppid == running->pid) {
			proc[i].ppid = proc[1].pid;
		}
	}
	wakeup(&proc[running->ppid]);
	tswitch();
}
// sleep a process
void sleep(PROC* event)
{
	running->event = event;
	running->status = SLEEP;
	enqueue(running, &sleepList);

	tswitch();
}
// make a process wait
int wait(int *status){
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
		sleep(running);
	}
}

// set running proc's status to dead and switch to next proc
int grave(){
	printf("*****************************************\n");
	printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
	printf("*****************************************\n");
	running->status = DEAD;

	tswitch();   /* journey of no return */
}
// print processes
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

// basically main
int body(){
	char c;
	int status, pid;
	// where the program rests
	while(1){
		ps();
		printf("\nProc %d[%d]\n", running->pid, running->ppid);
		printf("Input a char : [[f]ork|[z]sleep|[p]rint|[k]ill|[s]witch|wake[u]p|[w]ait] \n");
		c=getc();
		switch(c){
			case 'f': kfork();
					  printf("\n");
					  break;
			case 'z': printf("event: ");
					  c = getc();
					  printf("\n");
					  status = c - '0';
					  sleep(status);
					  printf("\n");
					  break;
			case 'p': printQueue(running);
					  printf("\n");
					  break;
			case 'k': printf("input exitVal: ");
					  c = getc();
					  c = c - '0';
					  printf("%c\n", c);
					  kexit((int)c);
					  printf("\n");
					  break;
			case 's': tswitch();
					  printf("\n");
					  break;
			case 'u': printf("event: ");
					  c = getc();
					  printf("\n");
					  status = c - '0';
					  wakeup(status);
					  printf("-----------\n");
					  break;
			case 'w': pid = wait(&status);
					  if (pid < 1)
						  printf("ERR NO CHILDREN\n");
					  else
						  printf("pid=%d exitVal=%d\n",pid,status);
					  printf("-----------\n");
					  break;
			default : printf("-----------\n");
					  break;
		}
	}
}
// basic prints and begin switching and forking
main(){
	printf("\nWelcome to the 460 Multitasking System\n");
	initialize();
	kfork();
	printf("P0 switch to P1\n");
	tswitch();
	printf("P0 resumes: all dead, happy ending\n");
}
// queues the running process and dequeues the next one
int scheduler(){
	if (running->status == READY)
		enqueue(running, &readyQueue);

	running = dequeue(&readyQueue);
	printf("------------\n");
	printf("next running proc = %d\n", running->pid);
	printf("------------\n");
}
// return a the top fre process or return NULL
PROC* getproc(){
	if (freeList != NULL)
		return dequeue(&freeList);
	return freeList;
}
// setup the stacks of 9 processes to be populated
int kfork(){
	PROC *p = getproc();
	int i;

	if (p==NULL){
		printf("ERROR: no free procs\n");
		return -1;
	}
	p->ppid = running->pid;
	p->status = READY;
	for (i=1; i<10; i++){
		p->kstack[SSIZE - i] = 0;          // all saved registers = 0
		p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
		p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
	}

	enqueue(p, &readyQueue);
	//printf("kfork()");
	return p->pid;
}
