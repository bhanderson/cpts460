myprints(char *s)
{
	while(*s)
		putc(*s++);
}

mygets(char *s)
{
	char c;
	while ( (c = getc()) != '\r'){
		*s++ = c;
		putc(c);
	}
	*s = 0;
}

char ans[64];

main()
{
	while(1){
	myprints("Enter a filename to boot: ");
	mygets(ans);
	myprints("\n\r");
	if (strcmp(ans, "quit")==0)
		break;
	}
/*
	while(1){
		myprints("what's your name? ");
		mygets(ans);
		myprints("\n\r");
		if (strcmp(ans, "quit")==0)
			break;
		myprints("Welcome ");
		myprints(ans);
		myprints("\n\r");

	}*/
}

