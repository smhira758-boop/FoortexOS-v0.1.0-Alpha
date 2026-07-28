void kmain(void) {
	volatile char* vga_buffer = (volatile char*) 0xB8000;
	const char* message = "Hello this is foortex OS ,--made by hira!";

	for (int i =0 ; message[i] != '\0'; i++) {
	     vga_buffer[i* 2]=message[i] ;
	     vga_buffer[i * 2+1] = 0x0f;
	}

	while(1) ;
}

