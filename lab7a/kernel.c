int enqueue(PROC **queue, PROC *p){
	PROC *current = 0, *prev = 0;
	
	if (*queue == 0){
		*queue = p;
		p->next = 0;
	} else if (p->pri > (*queue)->pri){
		p->next = (*queue);
		*queue = p;
	} else {
		current = prev = *queue;
		while(current && current->pri >= p->pri){
			prev = current;
			current = current->next;
		}
		prev->next = p;
		p->next = current;
	}
	return 1;
}

int dequeue(PROC **queue){
	PROC *p = 0;

	if (*queue == 0){
		p = 0;
	} else {
		p = *queue;
		*queue = (*queue)->next;
	}
	printf ("dequeued: %d", p);
	return p;
}

PROC *get_proc(PROC **list){
	return (dequeue(list));
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
	printf("%s\n",filename);
	load(filename, segment);
	printf("loaded %s at %u\n", filename, segment);
	printf("hello world\n");
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
	running->exitValue = val;
	running->status = ZOMBIE;

	wakeup(&proc[running->ppid]);
	tswitch();
}

int printQueue(PROC *list){
	PROC *p = list;
	while(p != 0){
		printf("{%d}=>", p->pid);
		p = p->next;
	}
	printf("0\n");
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
