void enqueue(PROC **queue, PROC *p){
	PROC *past;
	PROC *temp;

	past = *queue;
	if (past == NULL || p->pri > past->pri){
		p->next = past;
		*queue = p;
		return;
	}
	temp = past->next;
	while (temp != NULL && temp->pri >= p->pri ){
		past = past->next;
		temp = temp->next;
	}
	past->next = p;
	p->next = temp;
}

PROC *dequeue(PROC **queue){
	PROC *p = *queue;
	if (p!=0)
		*queue = p->next;
	//printf("dequeued proc #%d\n", p->pid);
	return p;
}

PROC * get_proc(PROC **list){
	return dequeue(list);
}

PROC *getproc(){
	return get_proc(&freeList);
}

void printQueue(PROC *queue){
	PROC *p = queue;
	while(p!=NULL){
		printf("[%d]->",p->pid);
		p = p->next;
	}
	printf("\n");
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
	p->pri = 1;                 // all of the same priority 1
	// clear kstack
	for (i = 1; i < 10; i++) {
		p->kstack[SSIZE -i] = 0;
	}
	p->kstack[SSIZE -1] =(int)body;
	p->ksp = &(p->kstack[SSIZE-9]);
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
int goUmode();

int copyImage(u16 s1,u16 s2, u16 size)
{
	// your copyimage function
	int i;
	for (i = 0; i < size; i+=2) {
		put_word(get_word(s1, i), s2, i);
	}
}


int fork()
{
	PROC *p;
	int  i, child;
	u16  segment;

	/*** get a PROC for child process: **/
	if ( (p = get_proc(&freeList)) == NULL){
		printf("no more proc\n");
		return(-1);
	}
	printf("ufork\n");
	/* set procs values to running and ready so we can use it */
	p->status = READY;
	p->next = NULL;
	p->ppid = running->pid;
	p->parent = running;
	p->pri = running->pri;

	/* zero out kstack registers*/
	for (i = 1; i < 10; i++) {
		p->kstack[SSIZE -i] = 0;
	}
	// set address to resume to
	p->kstack[SSIZE -1] =(int)goUmode;
	p->ksp = &(p->kstack[SSIZE-9]);

	// set segment to processes data position
	segment = (p->pid + 1)*0x1000;
	// copy the segment
	copyImage(running->uss, segment, 32*1024);
	printf("loaded at %u\n", segment);
	// clean the registers and set flag and uCs and uDs to runnings values
	for (i = 1; i < 13; i++) {
		child = 0x1000 - i*2;
		switch(i){
			case 1: put_word(segment, segment, child); break;
			case 2: put_word(segment, segment, child); break;
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10: put_word(0,segment, child); break;
			case 11:
			case 12: put_word(segment, segment, child); break;
		}
	}
	// same as kfork
	p->uss = segment;
	p->usp = 0x1000 - 24;
	put_word(0, segment, p->usp + 8*2);

	printf("Proc%d forked a child %d segment=%x\n", running->pid,p->pid,segment);
	enqueue(&readyQueue, p);
	/*
	   for (i=0; i<NFD; i++){
	   p->fd[i] = running->fd[i];
	   if (p->fd[i] != 0){
	   p->fd[i]->refCount++;
	   if (p->fd[i]->mode == READ_PIPE)
	   p->fd[i]->pipe_ptr->nreader++;
	   if (p->fd[i]->mode == WRITE_PIPE)
	   p->fd[i]->pipe_ptr->nwriter++;
	   }
	   }
	   */
	return(p->pid);
}
int kwait(){
	int i;
	wait(i);
	return i;
}

int wait(int *val){
	PROC *p;
	int i, child;
	while (1) {
		child = 0;
		for (i = 0; i < NPROC; i++) {
			p = &proc[i];
			if (p->ppid == running->pid && p->status != FREE){
				child++;
				if (p->status == ZOMBIE){
					p->status = FREE;
					*val = p->exitCode;
					p->next = freeList;
					freeList = p;
					return(p->pid);
				}
			}
		}
		if (!child)// no children
			return -1;
		// if children sleep on self
		sleep(running);
	}
}
int sleep(int event){
	running->event = event;
	running->status = SLEEP;
	enqueue(&sleepList, running);
	//enqueue(running, &sleepList);
	tswitch();
}


int wakeup(int event){
	PROC *p = sleepList;
	PROC *t = sleepList;
	if (p == NULL)
		return -1;
	while (p != NULL){
		if (p->event == event){
			p->status = READY;
			t->next = p->next;
			if (p == sleepList)
				sleepList = p->next;
			enqueue(&readyQueue,p);
			return;
		} else{
			t = p;
			p = p->next;
		}
	}
}


int kexit(int *val){
	int i = 0;
	if (running->pid == proc[1].pid)
		for (i = 2; i < NPROC; i++) {
			if (proc[i].status == READY){
				printf("proc %d not free\n", i);
				return -1;
			}
		}
	for (i = 1; i < NPROC; i++) {
		if (proc[i].ppid == running->pid){
			proc[i].ppid = 1;
			if (proc[i].status == ZOMBIE)
				wakeup(&proc[1]);
		}
	}
	running->exitCode = val;
	running->status = ZOMBIE;

	wakeup(&proc[running->ppid]);
	tswitch();
}

int kexec(char *filename)
{
	// your exec function
	// basically do the same initialization as kfork but with different path
	char name[128];
	int i, child;
	u16 segment = (running->pid +1) * 0x2000;
	// get the filename from umode
	for (i = 0; i < 128; i++) {
		name[i] = get_byte(segment, filename + i);
		if (name[i] == '\0')
			break;
	}
	// load the file
	load(name, segment);
	// clear registers except flag, uDS uES and uCS
	for (i = 1; i < 13; i++) {
		switch(i){
			case 1:		child = 0x0200;		break;
			case 2:
			case 11:
			case 12:	child = segment;	break;
			case 10:	put_word(0, segment, 0x2000-i*2); continue;
			default:	child = 0;			break;
		}
		put_word(child, segment, 0x2000-i*2);
	}
	// set uss to file position and set usp ustack top position
	running->uss = segment;
	running->usp = 0x2000-24;
	put_word(0, segment, running->usp + 8*2);
	return 1;

}

