#include "isr.h"
#include "keyboard.h"
#include "fifo.h"

fifo_t keyfifo;
uint32_t keybuf[32];
extern uint32_t keymap[];

static uint8_t get_scancode()
{
    uint8_t scancode;
    asm("cli");
    scancode = fifo_get(&keyfifo);
    asm("sti");
    return scancode;
}

static void keyboard_read()
{
    uint8_t scancode;
    int make;
    if (fifo_status(&keyfifo) > 0) {
        scancode = get_scancode();
        if (scancode == 0xE1) {
            // 特殊开头，暂不做处理
        } else if (scancode == 0xE0) {
            // 特殊开头，暂不做处理
        } else {
            make = (scancode & FLAG_BREAK ? true : false);
            if (make) {
                char key = keymap[(scancode & 0x7f) * MAP_COLS];
                monitor_put(key);
            }
        }
    }
}

void keyboard_handler(registers_t *regs)
{
    fifo_put(&keyfifo, inb(KB_DATA));
    keyboard_read();
}

void init_keyboard()
{
    fifo_init(&keyfifo, 32, keybuf);
    register_interrupt_handler(IRQ1, keyboard_handler);
}