// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
//
// This program only needs to handle arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.

// pseudo code:
// int i   = 0;
// int r0  = input;
// int r1  = input;
// int r2 = 0;
// while( 1 ) {
//     if( r0 - r1 > 0 ) { // LOOP0
//         r2 += r1;
//         i++
//         if( r0 - i <= 0 ) { break; }
// }
//     else { // LOOP1
//         r2 += r0;
//         i++;
//         if( r1 - i <= 0 ) { break; }
//    }
// }
// 

// Put your code here.

//  INIT i = 0, r2 = 0, if r0, r1 == 0
    @i
    M = 0

    @R2
    M = 0

    @R0
    D = M
    @END
    D;JEQ

    @R1
    D = M
    @END
    D;JEQ

    @R0
    D = M - D // R0 - R1
    @LOOP1
    D;JGT
    

(LOOP0)
    // R2+=R1
    @R1
    D = M     // D = R1

    @R2
    M = M + D // R2 = R2 + R1

    @i
    M = M + 1 // i++
    D = M // D = i

    // if( r0 - i <= 0 )
    @R0
    D = M - D // D = r0 - i

    @END
    D;JLE

    @LOOP0
    0;JMP

(LOOP1)
    // R2+=R0
    @R0
    D = M     // D = R0

    @R2
    M = M + D // R2 = R2 + R0

    @i
    M = M + 1 // i++
    D = M // D = i

    // if( r1 - i <= 0 )
    @R1
    D = M - D // D = r1 - i

    @END
    D;JLE

    @LOOP1
    0;JMP

(END)
    @END
    0;JMP