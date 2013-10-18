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
	PROC *p;  int i, child, pid;  u16 segment;

	pid = kfork(0);   // kfork() but do NOT load any Umode image for child
	if (pid < 0){     // kfork failed
		return -1;
	}
	p = &proc[pid];   // we can do this because of static pid

	for (i=0; i<NFD; i++){
		p->fd[i] = running->fd[i];

		if (p->fd[i] != 0){
			p->fd[i]->refCount++;

			if (p->fd[i]->mode == READ_PIPE)
				p->fd[i]->pipe_ptr->nreader++;

			if (p->fd[i]->mode == WRITE_PIPE){
				p->fd[i]->pipe_ptr->nwriter++;
			}
		}
	}

	segment = (pid+1)*0x2000;
	copyImage(running->uss, segment, 32*1024);
	p->uss = segment;
	p->usp = 0x2000 - 24;

	// YOUR CODE to make the child runnable in User mode
	p->kstack[SSIZE -1] =(int)goUmode;
	/**** ADD these : copy file descriptors ****/

	// clean the registers and set flag and uCs and uDs to runnings values
	for (i = 1; i < 13; i++) {
		child = 0x2000 - i*2;
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

	return(p->pid);
}

extern int loader();

int exec(char *filename)
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

