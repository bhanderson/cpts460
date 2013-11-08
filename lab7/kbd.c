#define KEYBD	0x60
#define PORT_B	0x61
#define KBIT	0x80
#define KBSIZE	64
#define N_SCAN	105

char unshift[N_SCAN] = {
	0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
	'q','w','e','r','t','y','u','i',      'o','p','[',']', '\r', 0,'a','s',
	'd','f','g','h','j','k','l',';',       0, 0, 0, 0,'z','x','c','v',
	'b','n','m',',','.','/', 0,'*',        0, ' '
};

/* Scan codes to ASCII for shifted keys; unused keys are left out */
char shift[N_SCAN] = {
	0,033,'!','@','#','$','%','^',        '&','*','(',')','_','+','\b','\t',
	'Q','W','E','R','T','Y','U','I',      'O','P','{','}', '\r', 0,'A','S',
	'D','F','G','H','J','K','L',':',       0,'~', 0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',         0, ' '
};

int kbdata;            // has KBD input flag
char kbc;              // data char
char inbuf[128];
int head, tail, data, shifted;


int kbinit()
{
	//printf("kbinit() : ");
	kbdata = 0;          // flag = 0 initially
	head = tail = data = shifted = 0;
	enable_irq(1);
	out_byte(0x20, 0x20);
	//printf("kbinit done\n\r");
}

/************************************************************************
  kbhandler() is the kbd interrupt handler. PC's kbd generates 2 interrupts
  for each key typed; one when the key is pressed and another one when the
  key is released. Each key generates a scan code. The scan code of a key
  release is 0x80 + the scan code of key pressed. When the kbd interrupts,
  the scan code is in the data port (0x60) of the KBD interface. First,
  read the scan code from the data port. Then ack the key input by strobing
  its PORT_B at 0x61.
  Some special keys generate ESC key sequences,e.g. arrow keys.

  Then process the scan code:
  1. Ignore key releases, thus no special keys, such as shift.
  For normal keys: translate into LOWER CASE ASCII char.
  2. Raise the kbdata flag, allowing a busy waiting process to get the key
 ************************* Lower half driver ******************************/
int kbhandler()
{
	int scode = 0, value = 0, c = 0;

	/* Fetch the character from the keyboard hardware and acknowledge it. */
	scode = in_byte(KEYBD);/* get the scan code of the key struck */
	value = in_byte(PORT_B);/* strobe the keyboard to ack the char */
	out_byte(PORT_B, value | KBIT);/* first, strobe the bit high */
	out_byte(PORT_B, value); /* then strobe it low */
// if the shift key is down
	if (shifted == 1){
		kbc = shift[scode];
	}else{
		kbc = unshift[scode];
	}
// if the shift key is released right shift or left shift
	if (scode == 0xAA | scode == 0xB6){
		shifted --;
	}
// if the shift key is pressed down right or left shift
	if (scode == 0x2A | scode == 0x36){
		shifted = 1;
		goto out;
	}

	//ignore other up keys
	if (scode & 0x80) // ignore key up
		goto out;

	if(data != 128){ // if the buffer has room
		// put the data in incrememnt the pointers and mod them to get the loop
		// buffer
		inbuf[head] = kbc;
		printf("%s",inbuf);
		head++;
		head%=128;
		data++;
		// wakeup when it gets data
		wakeup(&inbuf);
	} else {
		//printf("sound beep\n");
	}

	printf("\n%c\n",kbc);
	//printf("kb interrupt %x %c %d\n", scode, kbc, running->inkmode);  // should see the scand code

	// if in umode
	if (running->inkmode == 0){
		//	printf("in umode");

	}
	kbdata = 1;                          // raise kbdata flag

out:
	// set the interrupt back on
	out_byte(0x20, 0x20);
}

/**************************** upper half driver ***********************/
// process call getc() to return a char from KBD, by busy waiting loop
int getc()
{
	char c;

	unlock();          // syscall to MTX kernel MAY have masked out interrupts
	if (data == 0){
		//printf("sleeping till data");
		// sleep if there is no data
		sleep(&inbuf);
	}
	// dont want your data to change while getting a char
	lock();
	// same circular buffer code
	c = inbuf[tail];
	tail ++;
	tail %= 128;
	data --;
	unlock();// dont forget to unlock the interrupts
	return c;
}

