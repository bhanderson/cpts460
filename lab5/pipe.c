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
}

int write_pipe(int fd, char *buf, int n)
{
	// your code for write_pipe()
}

int kpipe(int pd[2])
{
	// create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
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
