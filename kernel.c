static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void update_hardware_cursor(int row, int col) {
    unsigned short position = (row * 80) + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void enable_hardware_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 5); 
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15); 
}

const char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
};

void draw_foortex_logo(volatile char* vga, int start_row, int start_col) {
    const char* logo[18] = {
        "     .---.     ",
        "    /     \\    ",
        "   |   .   |   ",
        "   |  / \\  |   ",
        "    \\  _/  /   ",
        "     `---'     ",
        "       /       ",
        "   .--/---.    ",
        "  (  /     )   ",
        "   `|`----'    ",
        "    | /        ",
        "    |/         ",
        "    |\\         ",
        "    | \\        ",
        "  --'  \\       ",
        " (      )      ",
        "  `----'       ",
        "    |          "
    };

    for (int r = 0; r < 18; r++) {
        for (int c = 0; logo[r][c] != '\0'; c++) {
            if (logo[r][c] != ' ') {
                int offset = ((start_row + r) * 80 + (start_col + c)) * 2;
                vga[offset]     = logo[r][c];
                vga[offset + 1] = 0x0F; 
            }
        }
    }
}

void kmain(void) {
    volatile char* vga = (volatile char*) 0xB8000;
    
    for (int i = 0; i < 2000 * 2; i += 2) {
        vga[i]     = ' ';
        vga[i + 1] = 0x03; 
    }

    const char* title = "=== FOORTEX OS WORKSPACE ===";
    for (int i = 0; title[i] != '\0'; i++) {
        vga[i * 2]     = title[i];
        vga[i * 2 + 1] = 0x0B; 
    }

    const char* sub = "Type anywhere -- Use Arrow Keys to move live cursor!";
    for (int i = 0; sub[i] != '\0'; i++) {
        vga[160 + i * 2]     = sub[i];
        vga[160 + i * 2 + 1] = 0x0F;
    }

    draw_foortex_logo(vga, 3, 52);

    enable_hardware_cursor();

    int row = 3;
    int col = 0;
    update_hardware_cursor(row, col);

    while (1) {
        if (inb(0x64) & 1) {
            unsigned char scancode = inb(0x60);
            
            if (scancode < 0x80) {
                if (scancode == 0x48) { 
                    if (row > 3) row--;
                } else if (scancode == 0x50) { 
                    if (row < 24) row++;
                } else if (scancode == 0x4B) { 
                    if (col > 0) col--;
                } else if (scancode == 0x4D) { 
                    if (col < 48) col++;
                } 
                else if (scancode == 0x1C) { 
                    row++;
                    col = 0;
                } else if (scancode == 0x0E) { 
                    if (col > 0) {
                        col--;
                        int offset = (row * 80 + col) * 2;
                        vga[offset]     = ' ';
                        vga[offset + 1] = 0x03;
                    }
                } 
                else {
                    char ch = scancode_map[scancode];
                    if (ch != 0) {
                        int offset = (row * 80 + col) * 2;
                        vga[offset]     = ch;
                        vga[offset + 1] = 0x0A; 
                        col++;
                        if (col >= 48) { 
                            col = 0;
                            row++;
                        }
                    }
                }
                
                update_hardware_cursor(row, col);
            }
        }
    }
}
