/* timer parameters. */
#define LATCH_COUNT     0x00	/* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	/* ccaammmb, a = access, m = mode, b = BCD */

/************************ NOTICE THE DIVISOR VALUE ***********************/
#define TIMER_FREQ   1193182L	/* timer frequency for timer in PC and AT */
#define TIMER_COUNT ((unsigned) (TIMER_FREQ/60)) /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0


int enable_irq(irq_nr) unsigned irq_nr;
{
	lock();
	out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}

/*===========================================================================*
 *				timer_init				     *
 *===========================================================================*/

ushort tick, sec, min ,hr;
ushort floppy, procsleep;
PROC *p;
int temprow, tempcol;
int timer_init()
{
	/* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

	printf("timer init\n");
	tick = sec = min = hr = floppy = procsleep = 0;

	out_byte(TIMER_MODE, SQUARE_WAVE);	/* set timer to run continuously */
	out_byte(TIMER0, TIMER_COUNT);	/* load timer low byte */
	out_byte(TIMER0, TIMER_COUNT >> 8);	/* load timer high byte */
	enable_irq(TIMER_IRQ);
}

/*===========================================================================*
 *				timer_handler				     *
 *===========================================================================*/

int thandler()
{
	tick++;
	tick %= 60;

	if (tick % 60 == 0){
		// store rows and columns
		temprow = row;
		tempcol = column;
		row = 24;
		column = 68;
		sec++;
		// increment minutes and hours acccordingly
		if (sec==60)
			min++;
		if (min==60)
			hr++;
		if(hr<10)
			printf("0");
		printf("%d:", hr);
		if(min<10)
			printf("0");
		printf("%d:", min%=60);
		if(sec<10)
			printf("0");
		printf("%d", sec%=60);
		//printf("%d", sleepList->time);
		//printf("%d:%d:%d", hr, min%=60, sec%=60);
		row = temprow;
		column = tempcol;
		// decrement runnings time so its not takign all the work
		if (running->time>=1 && inkmode == 1)
			running->time--;
		//printf("running->time=%d", running->time);
		// a proc is asleep so decrement procsleep
		if (sleepList!=0){
			p = sleepList;
			// still in sleepList
			while(p){
				// if proc still has time left decrement time
				if (p->time > 0)
					p->time --;
				// if proc is ready to be waked up wake up
				if (p->time <= 0 && p->event == p->pid)
					wakeup(p->pid);
				// else go to next proc in sleeplist
				p = p->next;
			}
		}


		}
		// switch procs at set value
		if (running->time == 0 && inkmode==1){
			out_byte(0x20, 0x20);
			do_switch();
			running->time=10;
		}
		// if floppy drive is on turn on light
		if (floppy==1 && sec%5==0)
			out_byte(0x0C, 0x3F2);
		// if floppy drive is off turn off light
		if (floppy==0 && sec%5==0)
			out_byte(0x1C, 0x3F2);



		out_byte(0x20, 0x20);

	}





