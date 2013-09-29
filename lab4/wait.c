int sleep(int event){
	running->event = event;
	running->status = SLEEP;
	enqueue(&sleepList, running);
	//enqueue(running, &sleepList);
	tswitch();
}

int wait(int *val){
	PROC *p;
	int i, child;
	while (1) {
		child = 0;
		for (i = 0; i < NPROC; i++) {
			p = &proc[i];
			if (p->ppid == running->pid && p->status != FREE){
				child=1;
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
int do_exit(int *val){
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

