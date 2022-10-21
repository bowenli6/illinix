#include <drivers/terminal.h>
#include <io.h>


/* Local functions, see headers for descriptions. */

static void output(uint32_t scancode, uint8_t caps);
static void backspace();

/**
 * @brief Initialize the terminal.
 * 
 */
void terminal_init() {
    terminal.capslock = 0;              /* CapsLock is not pressed. */
    terminal.shift = 0;                 /* Shift is not pressed. */
    terminal.ctrl = 0;                  /* Ctrl is not pressed. */
    // memset((void*)terminal.buffer, 0, KEYBOARD_SIZE);
}




/**
 * @brief Parse key when the a key is pressed.
 * 
 * @param scancode : The scancode of the key.
 */
void key_press(uint32_t scancode) {
    puts("   ");
    printf("%d", scancode);
    puts("   ");
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
    case BACKSPACE:
        backspace(scancode);
        return;
    case CTRL:
        terminal.ctrl = 1;
        return;
    case L:
        if (terminal.ctrl)                      /* If ctrl is hold and CTRL-L is pressed. */
            clear();                            /* Clear the screen.*/
        return;
    default:
        if (terminal.shift)                     /* If Shift is hold, output in capital form. */
            output(scancode, 1);
        else                                    /* Otherwise, output in capslock from. */
            output(scancode, terminal.capslock);
        break;
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
 * @brief Print the character that specified by the given keyboard scancode.
 * 
 * @scancode: The keyboard scancode of the input character.
 * @caps: set to 1 when the character should be printed in capital form.
 */
static void output(uint32_t scancode, uint8_t caps) {
    if (scancode >= KEYBOARD_SIZE) return;          /* Should not print. */
    uint8_t character = scancodes[scancode][caps];  /* Get character. */
    if (character) putc(character);     
}

/**
 * @brief Handle the terminal screen when backspace key is pressed. 
 * 
 */
static void backspace() {

    // TODO : There is a BUG when backspace is pressed!

    /* No way to backspace. */
    if (screen_x == 0 && screen_y == 0) {
        return;
    } else if (screen_x == 0 && screen_y != 0) {
        screen_x = NUM_COLS - 1;            /* Go back to the previous position. */
        --screen_y;
        putc(' ');
        screen_x = NUM_COLS - 1;            /* Go back to the previous position again */
        --screen_y;                         /* because puc will increase them. */
    } else {
        --screen_x;                         /* Go back to the previous position. */
        putc(' ');
        --screen_x;                         /* Go back to the previous position again */
    }
}

int32_t terminal_open(const int8_t *fname) {
    return 0;
}

int32_t terminal_close(int32_t fd) {
    return 0;
}

int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes) {
    return 0;
}

int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes) {
    return 0;
}
