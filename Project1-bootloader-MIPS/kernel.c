char *value = "2019";

void __attribute__((section(".entry_function"))) _start(void)
{
	// Call PMON BIOS printstr to print message "Hello OS!"
	void (*printstr)(char *) = (void *)0x80011100;
	printstr("Hello OS Version: ");
	printstr(value);
	while(1)
		;
	return;
}
