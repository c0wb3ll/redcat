[ BITS 64 ]

SECTION .text

global ExecuteSystemCall

ExecuteSystemCall:
    push rcx
    push r11

    syscall

    pop r11
    pop rcx
    ret