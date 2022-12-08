#include <drivers/terminal.h>
#include <vfs/vfs.h>
#include <pro/cfs.h>
#include <pro/process.h>
#include <kmalloc.h>
#include <lib.h>
#include <access.h>
#include <drivers/vga.h>
#include <io.h>
#include <boot/x86_desc.h>
#include <boot/page.h>

/* Local functions, see headers for descriptions. */

static void in(uint32_t scancode, uint8_t caps, terminal_t *terminal);
static void out(const void *buf, int32_t nbytes);
static void out_tab(uint32_t n, terminal_t *terminal);
static void backspace(terminal_t *terminal);
static void bufcpy(void *dest, const void *src, uint32_t nbytes, uint8_t bufhd);
static int isletter(uint32_t scancode);
static inline void terminal_switch(uint32_t scancode, terminal_t *terminal, int idx);



/* 1 when terminal driver is booted */
int8_t terminal_boot = 0;


/**
 * @brief create and initialize the terminal.
 * 
 */
terminal_t *terminal_create(void) {
    /* create a new terminal */
    terminal_t *terminal = kmalloc(sizeof(terminal_t));

    /* init terminal state */
    terminal->capslock = 0;                     /* capsLock is not pressed. */
    terminal->shift = 0;                        /* shift is not pressed. */
    terminal->ctrl = 0;                         /* ctrl is not pressed. */
    terminal->alt = 0;                          /* alt key is not pressed */
    terminal->bufhd = 0;                        /* 0 characters read. */
    terminal->buftl = 0;                        /* 0 characters read. */
    terminal->size = 0;                         /* No character yet. */
    terminal->exit = 0;                         /* \n is not read. */
    terminal->buffer = kmalloc(TERBUF_SIZE);    /* create buffer */
    // terminal->saved_vidmem = VIDEO_BUF_1 + i*TERBUF_SIZE;       /* create video memory */
    // terminal->vidmem = terminal->saved_vidmem;  /* save back up video memory */
    // memset((void*)terminal->buffer, 0, TERBUF_SIZE);
    // vga_clear(terminal->vidmem);
    return terminal;
}


/**
 * @brief free the terminal.
 * 
 */
void terminal_free(terminal_t *terminal) {
    if (!terminal) return;
    kfree(terminal->buffer);
    kfree(terminal);
}


/**
 * @brief Parse key when the a key is pressed.
 * 
 * @param scancode : The scancode of the key.
 */
void key_press(uint32_t scancode, terminal_t *terminal) {
    switch (scancode) {
    case CAPSLOCK:
        terminal->capslock = !(terminal->capslock);  /* Reverse capslock. */
        return;
    case LSHIFT:
        terminal->shift = 1;                         
        return;
    case RSHIFT:
        terminal->shift = 1;
        return;
    case LALT:
        terminal->alt = 1;
    case BACKSPACE:
        backspace(terminal);
        return;
    case CTRL:
        terminal->ctrl = 1;
        return;
    case TAB:
        out_tab(TAB_SPACE, terminal);
    case F1:
        if (terminal->alt) {        
            terminal_switch(scancode, terminal, 0);
            return;
        }
        if (terminal->shift) {                    /* If Shift is hold, output in capital form. */
            in(scancode, 1 - (terminal->capslock & isletter(scancode)), terminal);
        } else {
            /* Otherwise, output in capslock from. */
            in(scancode, (terminal->capslock & isletter(scancode)), terminal);
        }                             
        return;
    case F2:
        if (terminal->alt) {    
            terminal_switch(scancode, terminal, 1);
            return;
        }
        if (terminal->shift) {                    /* If Shift is hold, output in capital form. */
            in(scancode, 1 - (terminal->capslock & isletter(scancode)), terminal);
        } else {
            /* Otherwise, output in capslock from. */
            in(scancode, (terminal->capslock & isletter(scancode)), terminal);
        }                             
        return;
    case F3:
        if (terminal->alt) {
            terminal_switch(scancode, terminal, 2);
            return;
        }
        if (terminal->shift) {                    /* If Shift is hold, output in capital form. */
            in(scancode, 1 - (terminal->capslock & isletter(scancode)), terminal);
        } else {
            /* Otherwise, output in capslock from. */
            in(scancode, (terminal->capslock & isletter(scancode)), terminal);
        }                             
        return;
    case L:
        if (terminal->ctrl) {                    /* If ctrl is hold and CTRL-L is pressed. */
            char buf[terminal->size];
            clear(); 
            puts("391OS> ");
            bufcpy((void*)buf, (void*)terminal->buffer, terminal->size, terminal->bufhd);
            out(buf, terminal->size);
        } else {
            if (terminal->shift) {                   
                in(scancode, 1 - (terminal->capslock & isletter(scancode)), terminal);
            } else {
                /* Otherwise, output in capslock from. */
                in(scancode, (terminal->capslock & isletter(scancode)), terminal);
            }      
        }
        return;
    default:
        if (terminal->shift) {                    /* If Shift is hold, output in capital form. */
            in(scancode, 1 - (terminal->capslock & isletter(scancode)), terminal);
        } else {
            /* Otherwise, output in capslock from. */
            in(scancode, (terminal->capslock & isletter(scancode)), terminal);
        }                             
        return;
    }
}

void switch_vidmap(int src, int dest)
{
    thread_t *prev, *next;
    prev = consoles[src]->task;
    next = consoles[dest]->task;

    vidmap_table[(VIDEO >> PDE_OFFSET_4KB) + src] = PTE_PRESENT | PTE_RW | PTE_US | ADDR_TO_PTE((uint32_t)consoles[src]->task->terminal->saved_vidmem);
    vidmap_table[(VIDEO >> PDE_OFFSET_4KB) + dest] = PTE_PRESENT | PTE_RW | PTE_US | ADDR_TO_PTE(VIDEO);
    

    flush_tlb();

}


static inline void terminal_switch(uint32_t scancode, terminal_t *terminal, int idx) {
    terminal_t *next_terminal;
    thread_t *next;

    if (scancode == current->fkey) return; 

    next = consoles[idx]->task;
    
    next_terminal = next->terminal;

    /* set its vidmem to back up */
    memcpy((void*)terminal->saved_vidmem, (void*)video_mem, VIDMEM_SIZE);
    terminal->vidmem = terminal->saved_vidmem;

    terminal->alt = 0;
    
    memcpy((void*)video_mem, (void*)next_terminal->saved_vidmem, VIDMEM_SIZE);

    if (next->state == UNUSED) {
        next->state = RUNNABLE;
        sched_fork(next);
        activate_task(next);
        vga_clear(video_mem);
    } 

    if (next->state == SLEEPING) {
        next->state = RUNNABLE;
        enqueue_task(next, 1);
    }   
    
    next_terminal->vidmem = video_mem;

    vga_update_cursor(next_terminal->screen_x, next_terminal->screen_y);
    
    switch_vidmap(current->id, idx);
        
    current = consoles[idx];
}

/**
 * @brief Parse key when the a key is release.
 * 
 * @param scancode : The scancode of the key.
 */
void key_release(uint32_t scancode, terminal_t *terminal) {
    switch (scancode) {
        case LSHIFT:
            terminal->shift = 0;             /* Uncheck shift. */
            break;
        case RSHIFT:
            terminal->shift = 0;             /* Uncheck shift. */
            break;
        case CTRL:
            terminal->ctrl = 0;              /* Uncheck ctrl. */
            break;
        case LALT:
            terminal->alt = 0;               /* uncheck alt */
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
static void in(uint32_t scancode, uint8_t caps, terminal_t *terminal) {
    if (scancode >= KEYBOARD_SIZE) return;          /* Should not print. */
    uint8_t character = scancodes[scancode][caps];  /* Get character. */
    if (character == '\r') character = '\n';
    if (character) {
        _putc(character, terminal);
        if (terminal->size  == TERBUF_SIZE)   
            terminal->bufhd = (terminal->bufhd + 1) % TERBUF_SIZE;

        terminal->buffer[terminal->buftl] = character;
        terminal->buftl = (terminal->buftl + 1) % TERBUF_SIZE;

        if (terminal->size != TERBUF_SIZE)   
            terminal->size++;            
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
    int i;
    char c;
    
    for (i = 0; i < nbytes; ++i) {
        c = ((char*)buf)[i];
        putc(c);
    }
}


/**
 * @brief Print n space when a tab is pressed.
 * 
 */
static void out_tab(uint32_t n, terminal_t *terminal) {
    int i;
    for (i = 0; i < n-(NUM_COLS % n); ++i) 
        in(SPACE, 0, terminal);  
}



/**
 * @brief Handle the terminal screen when backspace key is pressed. 
 * 
 */
static void backspace(terminal_t *terminal) {
    if (!terminal->size) {  
        /* No way to backspace. */
        return;
    } else {    
        /* Clear the most recent character. */
        back(terminal);
        terminal->size--;
        if (terminal->buftl == 0)
            terminal->buftl = TERBUF_SIZE - 1;
        else
            terminal->buftl--;
    }
}


/**
 * @brief Initialize terminal state. 
 * 
 * @param fname : stdin or stdout
 * @return int32_t : 0.
 */
int32_t terminal_open(const int8_t *fname) {
    return 0;
}


/**
 * @brief Clears any terminal specific variables. (do nothing for now.)
 * 
 * @param fd : 0 or 1.
 * @return int32_t : -1.
 */
int32_t terminal_close(int32_t fd) {
    return -1;
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
    thread_t *curr;
    terminal_t *terminal;

    curr = current->task;
    terminal = curr->terminal;

    if (!terminal) return -1;

    if (fd != stdin)
        return -1;

    if (nbytes < 0) 
        return -1;

    if (nbytes == 0)
        return 0;

    if (nbytes > TERBUF_SIZE)
        nbytes = TERBUF_SIZE;

    /* Init to zero. (read is not stopped) */
    terminal->exit = 0;

    while (!terminal->exit) {

        /* sleep and waiting for an IO operation */
        // sched_sleep(curr);

        /* Waiting for intrrupt occurs... */

        /* Critical section begins. */
        cli_and_save(intr_flag);

        for (nread = 0, start = terminal->bufhd; nread < terminal->size; nread++) {
            if ((terminal->buffer[start] == '\n') || (terminal->buffer[start] == '\r')) {
                nread++;
                terminal->exit = 1;
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
        bufcpy(buf, (void*)terminal->buffer, nbytes, terminal->bufhd);
        nread = nbytes;

    } else {
        /* normal case: the nread <= nbytes. */
        bufcpy(buf, (void*)terminal->buffer, nread, terminal->bufhd);
    }

    /* change the bufhd points to next part. */
    terminal->bufhd = (terminal->bufhd + nread) % TERBUF_SIZE;
    terminal->size -= nread;
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
    if (!buf)
        return -1;

    if (fd != stdout)
        return -1;
    
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
