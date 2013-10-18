show_pipe(PIPE *p)
{
	int i,j;
	printf("------------ PIPE CONTENTS ------------\n");
	// print pipe information
	printf("nreader=%d\tnwriter=%d\tdata=%d\troom=%d\ncontents=",
			p->nreader, p->nwriter, p->data, p->room);
	while(i != j && p->buf[i] != 0){
		printf("%c", p->buf[i]);
		i = ((i+1)%PSIZE);
	}
	printf("\n----------------------------------------\n");
}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
	// print running process' opened file descriptors
	int i = 0;
	OFT *op = running->fd[i];
	printf("=========== valid fd ===========\n");
	for (i = 0; i < NFD; i++) {
		op = running->fd[i];
		if(op->refCount!=0)
			printf("%d\t%s\trefCount = %d\n", i, MODE[op->mode - 4], op->refCount);
	}
	printf("=================================\n");
	return 0;
}

int read_pipe(int fd, char *buf, int n)
{
	// your code for read_pipe()

	int r = 0;
	PIPE *p = running->fd[fd]->pipe_ptr;
	if (n<=0) return 0;
	if (running->fd[fd]->mode == 1) return 0;
	if (running->fd[fd]==0) return -1;
	while(n){
		while(p->data){
			put_word(p->buf[ p->head ], running->uss, buf+r);
			p->head++;
			p->head = p->head % PSIZE;
			n--; r++; p->data--; p->room++;
			if (n==0) break;
		}
		if (n==0 || r){ // has data
			wakeup(&p->room);
			return r;
		}
		// pipe has no data
		if (p->nwriter){
			wakeup(&p->room);
			sleep(&p->data);
			continue;
		}
		// pipe has no writer and no data
		return 0;
	}

}

int write_pipe(int fd, char *buf, int n)
{
	// your code for write_pipe()
	int r = 0;
	char tmp;
	PIPE *p = running->fd[fd]->pipe_ptr;

	if (n<=0) return 0;
	if (running->fd[fd]->mode==0) return -1;
	printf("nreader, writer: %d\n", p->nreader);
	while (n){
		if (!p->nreader)
		{
			//do_exit(BROKEN_PIPE);
			printf("BROKEN_PIPE\n");
			close_pipe(fd);
			return 0;
		}
		while(p->room && n){
			tmp = get_word(running->uss, buf+r);
			//p->buf[ p->tail ] = buf[r];
			p->buf[ p->tail ] = tmp;
			p->tail++;
			p->tail = p->tail % PSIZE;
			r++; p->data++; p->room--; n--;
		}
		wakeup(&p->data);  // wakeup all readers, if any.
		if (n==0) return r; // finished writing n bytes
		// still has data to write but pipe has no room
		sleep(&p->room); // sleep for room
	}

	printf("end write\n");
	show_pipe(p);
}

int kpipe(int pd[2])
{
	int i, j, k;
	PIPE *p;
	for (i = 0; i < NPIPE; i++) {
		if(pipe[i].busy==0)
			break;
	}
	for (j = 0; j < NFD; j++) {
		if (running->fd[j] == 0)
			break;
	}
	for (k = 0; k < NOFT; k++) {
		if(oft[k].refCount==0){
			oft[k].refCount++;
			oft[k+1].refCount++;
			break;
		}
	}

	running->fd[j] = &oft[k];
	running->fd[j+1] = &oft[k+1];
	oft[k].pipe_ptr = &pipe[i];
	oft[k+1].pipe_ptr = &pipe[i];
	p = &pipe[i];

	p->head = p->tail = p->data = 0;
	p->room = PSIZE;
	p->nreader = 1;
	p->nwriter = 1;
	printf("nreader, kpipe: %d\n", p->nreader);
	p->busy = 1;
	oft[k].mode = READ_PIPE;
	oft[k+1].mode = WRITE_PIPE;
	if(running->fd[i]==0)
		return -1;
	put_word(j, running->uss, pd);
	put_word(j+1, running->uss, pd+1);
	printf("CREATED PIPE: \n");
	show_pipe(p);
	return 0;
}



int close_pipe(int fd)
{
	OFT *op; PIPE *pp;

	printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

	op = running->fd[fd];
	running->fd[fd] = 0;                 // clear fd[fd] entry

	if (op->mode == READ_PIPE){
		pp = op->pipe_ptr;
		pp->nreader--;                   // dec n reader by 1

		if (--op->refCount == 0){        // last reader
			if (pp->nwriter <= 0){         // no more writers
				pp->busy = 0;             // free the pipe
				return;
			}
		}
		wakeup(&pp->room);
		return;
	}

	if (op->mode == WRITE_PIPE){
		pp = op->pipe_ptr;
		pp->nwriter--;                   // dec nwriter by 1

		if (--op->refCount == 0){        // last writer
			if (pp->nreader <= 0){         // no more readers
				pp->busy = 0;              // free pipe also
				return;
			}
		}
		wakeup(&pp->data);
		return;
	}
}
