// execute switch and print what proc is switching
int do_tswitch(){
	printf("PID %d switching", running->pid);
	tswitch();
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
	for (i = 0; i < NFD; i++) {
		close_pipe(i);
	}

	wakeup(&proc[running->ppid]);
	tswitch();
}

int sleep(int event){
	running->event = event;
	running->status = SLEEP;
	enqueue(&sleepList, running);
	//enqueue(running, &sleepList);
	tswitch();
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


