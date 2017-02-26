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

// Put your code here.

(Loop)
  @SCREEN
  D=A
  @1
  M=D     // Store screen position in R1
  @8192
  D=A
  @2
  M=D     // Store the screen size in R2
  @0
  M=-1    // Set the fill bits to 1
  @24576  // Check keyboard
  D=M
  @Fill
  D;JGT   // Jump to fill the screen
  @0
  M=0     // Set the fill bits to 0
(Fill)
  @0
  D=M     // Get the fill bits
  @1
  A=M     // Set screen position
  M=D     // Draw to screen
  A=A+1   // Go to next word
  D=A
  @1
  M=D     // Store away next position in R1
  @2
  M=M-1
  D=M
  @Fill
  D;JGT   // Jump back to Fill if we need to keep filling
  @Loop
  D;JMP   // Jump back to beginning