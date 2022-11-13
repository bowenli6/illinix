#include <drivers/terminal.h>
#include <lib.h>
#include <io.h>


terminal_t terminal;    /* The terminal object. */


/* Local functions, see headers for descriptions. */

static void terminal_init();
static void in(uint32_t scancode, uint8_t caps);
static void out(const void *buf, int32_t nbytes);
static void out_tab();
static void backspace();
static void bufcpy(void *dest, const void *src, uint32_t nbytes, uint8_t bufhd);
static int isletter(uint32_t scancode);

/**
 * @brief Initialize the terminal.
 * 
 */
static void terminal_init() {
    terminal.capslock = 0;              /* CapsLock is not pressed. */
    terminal.shift = 0;                 /* Shift is not pressed. */
    terminal.ctrl = 0;                  /* Ctrl is not pressed. */
    terminal.alt = 0;                   /* alt key is not pressed */
    terminal.bufhd = 0;                 /* 0 characters read. */
    terminal.buftl = 0;                 /* 0 characters read. */
    terminal.size = 0;                  /* No character yet. */
    terminal.exit = 0;                  /* \n is not read. */
    memset((void*)terminal.buffer, 0, TERBUF_SIZE);
}



/**
 * @brief Parse key when the a key is pressed.
 * 
 * @param scancode : The scancode of the key.
 */
void key_press(uint32_t scancode) {
    switch (scancode) {
    case CAPSLOCK:
        terminal.capslock = !(terminal.capslock);  /* Reverse capslock. */
        return;
    case LSHIFT:
        terminal.shift = 1;                         
        return;
    case RSHIFT:
        terminal.shift = 1;
        return;
    case LALT:
        terminal.alt = 1;
    case BACKSPACE:
        backspace(scancode);
        return;
    case CTRL:
        terminal.ctrl = 1;
        return;
    case TAB:
        out_tab(TAB_SPACE);
    case L:
        if (terminal.ctrl) {                    /* If ctrl is hold and CTRL-L is pressed. */
            clear(); 
        } else {
            if (terminal.shift) {                   
                in(scancode, 1 - (terminal.capslock & isletter(scancode)));
            } else {
                /* Otherwise, output in capslock from. */
                in(scancode, (terminal.capslock & isletter(scancode)));
            }      
        }
        return;
    default:
        if (terminal.shift) {                    /* If Shift is hold, output in capital form. */
            in(scancode, 1 - (terminal.capslock & isletter(scancode)));
        } else {
            /* Otherwise, output in capslock from. */
            in(scancode, (terminal.capslock & isletter(scancode)));
        }                             
        return;
    }
}


/**
 * @brief Parse key when the a key is release.
 * 
 * @param scancode : The scancode of the key.
 */
void key_release(uint32_t scancode) {
    switch (scancode) {
        case LSHIFT:
            terminal.shift = 0;             /* Uncheck shift. */
            break;
        case RSHIFT:
            terminal.shift = 0;             /* Uncheck shift. */
            break;
        case CTRL:
            terminal.ctrl = 0;              /* Uncheck ctrl. */
            break;
        default:
            break;
    }
}

    
/**
 * @brief Store the character from stdin that specified by the given keyboard 
 * scancode into the terminal buffer.
 * 
 * @scancode: The keyboard scancode of the input character.
 * @caps: set to 1 when the character should be stored in capital form.
 */
static void in(uint32_t scancode, uint8_t caps) {
    if (scancode >= KEYBOARD_SIZE) return;          /* Should not print. */
    uint8_t character = scancodes[scancode][caps];  /* Get character. */
    if (character) {
        putc(character);
        if (terminal.size  == TERBUF_SIZE)   
            terminal.bufhd = (terminal.bufhd + 1) % TERBUF_SIZE;

        terminal.buffer[terminal.buftl] = character;
        terminal.buftl = (terminal.buftl + 1) % TERBUF_SIZE;

        if (terminal.size != TERBUF_SIZE)   
            terminal.size++;            
        /* otherwise the size does not change. (always as same as TERBUF_SIZE) */
    }
}

/**
 * @brief Print the character to stdout that specified by the given keyboard 
 * scancode.
 * 
 * @param buf : The buffer to print to the screen.
 * @param nbytes : The number of bytes need to print to the screen.
 */
static void out(const void *buf, int32_t nbytes) {
    /* The method of outputting data will be changed when connected with VGA. */
    int i;
    char c;
    
    for (i = 0; i < nbytes; ++i) {
        c = ((char*)buf)[i];
        putc(c);   /* output to the screen. */
    }
}


/**
 * @brief Print n space when a tab is pressed.
 * 
 */
static void out_tab(uint32_t n) {
    int i;
    for (i = 0; i < n-(NUM_COLS % n); ++i) 
        putc(' ');
}


/**
 * @brief Handle the terminal screen when backspace key is pressed. 
 * 
 */
static void backspace() {
    if (!terminal.size) {  
        /* No way to backspace. */
        return;
    } else {    
        /* Clear the most recent character. */
        back();
        terminal.size--;
        if (terminal.buftl == 0)
            terminal.buftl = TERBUF_SIZE - 1;
        else
            terminal.buftl--;
    }
}


/**
 * @brief Initialize terminal state. 
 * 
 * @param fname : stdin or stdout
 * @return int32_t : 0.
 */
int32_t terminal_open(const int8_t *fname) {
    terminal_init();
    return 0;
}


/**
 * @brief Clears any terminal specific variables. (do nothing for now.)
 * 
 * @param fd : 0 or 1.
 * @return int32_t : 0.
 */
int32_t terminal_close(int32_t fd) {
    return 0;
}


/**
 * @brief Read data from the stdin.
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file.
 * @param nbytes The number of bytes to read from the file.
 * @return int32_t -1 on failure (non-existent file or invalid inode number), 
 *                    number of bytes read on success.
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes) {
    uint32_t intr_flag;
    int32_t nread;
    uint8_t start = 0;

    if (nbytes < 0) 
        return -1;

    if (nbytes == 0)
        return 0;

    if (nbytes > TERBUF_SIZE)
        nbytes = TERBUF_SIZE;

    /* Init to zero. (read is not stopped) */
    terminal.exit = 0;

    while (!terminal.exit) {

        /* Waiting for intrrupt occurs... */

        /* Critical section begins. */
        cli_and_save(intr_flag);

        for (nread = 0, start = terminal.bufhd; nread < terminal.size; nread++) {
            if ((terminal.buffer[start] == '\n') || (terminal.buffer[start] == '\r')) {
                nread++;
                terminal.exit = 1;
                break;
            }
            start = (start + 1) % TERBUF_SIZE;
        }

        /* Critical section ends. */
        restore_flags(intr_flag);
    }

    /* new-line character has been detected! */
    /* When the input is larger than the given nbytes. */
    if (nread > nbytes) {
        /* copy nbytes from terminal buffer into user buffer. */
        bufcpy(buf, (void*)terminal.buffer, nbytes, terminal.bufhd);
        nread = nbytes;

    } else {
        /* normal case: the nread <= nbytes. */
        bufcpy(buf, (void*)terminal.buffer, nread, terminal.bufhd);
    }

    /* change the bufhd points to next part. */
    terminal.bufhd = (terminal.bufhd + nread) % TERBUF_SIZE;
    terminal.size -= nread;
    return nread;
}



/**
 * @brief Write data to stdout.
 * 
 * @param fd 
 * @param buf 
 * @param nbytes 
 * @return int32_t 
 */
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes) {
    uint32_t intr_flag;
    if (!buf) {
        puts("buf has NULL address.\n");
        return -1;
    }
    
    /* Critical section begins. */
    cli_and_save(intr_flag);

    out(buf, nbytes);

    /* Critical section ends. */
    restore_flags(intr_flag);

    return nbytes;
}



/**
 * @brief Copy nbytes number of bytes from the terminal buffer into
 * the given dest.
 * 
 * @param dest : Destination of copy
 * @param src : Source of copy
 * @param nbytes : Number of bytes to copy
 * @param bufhd : Start index.
 */
static void bufcpy(void *dest, const void *src, uint32_t nbytes, uint8_t bufhd) {
    int i;

    i = bufhd;
    while (nbytes--) {
        *(char*)dest++ = *(char*)(src + i);
        i = (i + 1) % TERBUF_SIZE;
    }

}

/**
 * @brief Check if the scancode represent a letter.
 * 
 * @param scancode : A scancode
 * @return int : 1 if true, 0 otherwise.
 */
static int isletter(uint32_t scancode) {
    return ((scancode >= 0x10 && scancode <= 0x19) || (scancode >= 0x1e && scancode <= 0x26)
        || (scancode >= 0x2c && scancode <= 0x32));
}
