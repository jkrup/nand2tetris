// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.


  @2
  M=0    // Yeah, set R2 to 0
(UPDOG)
  @1
  D=M    // D=R1
  @DOWNDOG
  D;JEQ  // Jump to end if R1 is zero (nothing more to add)
  @1
  M=M-1  // Decrement R1
  @0
  D=M    // Load R0 in D
  @2
  M=D+M  // Add R0+R2, load the result in R2
  @UPDOG // What's up dog?
  0;JMP  // Jump back
(DOWNDOG)
