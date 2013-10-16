show_pipe(PIPE *p)
{
	int i, j;
	printf("------------ PIPE CONTENETS ------------\n");
	// print pipe information
	printf("\n----------------------------------------\n");
}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
	// print running process' opened file descriptors
}

int read_pipe(int fd, char *buf, int n)
{
	// your code for read_pipe()
	int r = 0;
	if (n<=0) return 0;
	if (running->fd[fd]==0) return -1;
	PIPE *p = running->fd[fd]->pipe;
	char *data = p->buf;
	while(n){
		while(data){
			data[r] = buf[r];
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
	if (n<=0) return 0;
	if (running->fd[fd]==0) return -1;
	PIPE *p = running->fd[fd]->pipe;
	char *data = p->buf;
	while (n){
		if (!p->nreader)
			kexit(BROKEN_PIPE);
		while(p->room && n){
			p->buf[r] = buf[r];
			r++; p->data++; p->room--; n--;
		}
		wakeup(&p->data);  // wakeup all readers, if any.
		if (n==0) return r; // finished writing n bytes
		// still has data to write but pipe has no room
		sleep(&p->room); // sleep for room
	}

}

int kpipe(int pd[2])
{
	// create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
	PIPE p;
	OFT readOFT, writeOFT;
	int i;
	p.head = p.tail = p.data = 0;
	p.room = PSIZE;
	p.nreaders = p.nwriters = 1;
	readOFT.mode = READ_PIPE;
	writeOFT.mode = WRITE_PIPE;
	read_OFT.refCount = write_OFT.refCount = 1;
	read_OFT.pipe_ptr = write_OFT.pipe_ptr = &p;
	for (i = 0; i < NFD; i+2) {
		if(running->fd[i] == 0){
			running->fd[i] = &readOFT;
			running->fd[i+1] = &writeOFT;
			break;
		}
	}
	if(running->fd[i]==0)
		return -1;

	pd[0] = i;
	pd[1] = i+1;
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
