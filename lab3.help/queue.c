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
