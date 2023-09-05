// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// i = 0;
// while(1) {
//     if ( @24576 != 0 ) {
//         @16384 + i = -1;
//         if( i >= 8192 ) { continue; }
//         else { i++; }
//     } else {
//         @16384 + i = 0;
//         if( i <= 0 ) { continue; }
//         else { i--; }
//     }
// }

// Put your code here.

    @16384
    D = A

    @i
    M = D

(LOOP)
    @24576
    D = M

    @SETW
    D;JEQ

    @SETB
    0;JMP
    
(SETB)
    @i
    A = M
    M = -1
    D = A
    
    @16384
    D = D - A

    @8192
    D = A - D

    @LOOP
    D;JLE

    @i
    M = M + 1

    @LOOP
    0;JMP

(SETW)
    @i
    A = M
    M = 0
    D = A
    
    @16384
    D = D - A

    @LOOP
    D;JLE

    @i
    M = M - 1

    @LOOP
    0;JMP