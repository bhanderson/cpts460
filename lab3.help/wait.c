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
	//printf("dequeued proc %d\n", p->pid);
	if (*queue != NULL)
		*queue = (*queue)->next;
	return p;
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
int sleep(int event){
	printf("in sleep\n");
	running->event = event;
	running->status = SLEEP;
	enqueue(running, &sleepList);
	tswitch();
}

int wait(int *val){
	printf("in wait\n");
	PROC *p;
	int i, child;
	while (1) {
		child = 0;
		for (i = 0; i < NPROC; i++) {
			p = &proc[i];
			if (p->ppid == running->pid && p->status != FREE){
				child ++;
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

int wakeup(int event){
	printf("in wakeup\n");
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
			enqueue(p, &readyQueue);
			return;
		} else{
			t = p;
			p = p->next;
		}
	}
}
int do_exit(int *val){
	int i = 0;
	if (running->pid == proc[1].pid)
		for (i = 2; i < NPROC; i++) {
			if (proc[i].ppid == proc[1].pid && proc[i].status != FREE){
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

