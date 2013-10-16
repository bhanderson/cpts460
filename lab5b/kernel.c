
// run the kernel mode menu function
int kmode(){
	body();
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
		put_word(child, segment, 0x2000-i*2);
	}
	p->uss = segment;
	p->usp = 0x2000 - 12*2;

	printf("Proc%d forked a child %d segment=%x\n", running->pid,p->pid,segment);
	enqueue(&readyQueue, p);
	return(p->pid);
}

