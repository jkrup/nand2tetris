#[ Assembler.nim

   Converts a prog.asm hack assembly file to a prog.hack machine language file
   Usage: $> Assembler prog1.asm prog2.asm ...

   This file is notable because it is the first program I've written in Nim,
   as well as the first full program I've typed in Dvorak!
]#

import os, strutils, tables

proc isSymbol(token: string): bool =
  return isAlphaAscii(token[0])

proc stripLine(line: string): string =
  # first strip comments, then whitespace
  # note that all spaces are ignored
  let temp = line.split("//")
  return if temp.len() > 0: temp[0].strip().replace(" ")
         else:              line.strip().replace(" ")

proc initSymbols(symbols: var Table) =
  symbols["SP"]      = 0
  symbols["LCL"]     = 1
  symbols["ARG"]     = 2
  symbols["THIS"]    = 3
  symbols["THAT"]    = 4
  symbols["SCREEN"]  = 16384
  symbols["KBD"]     = 24576

  # do R0-R15
  for i in 0..15:
    symbols["R" & $i] = i

proc parseSymbols(line: string, symbols: var Table, linenum: var int) =
  let inst = stripLine(line)
  if inst.isNilOrWhitespace(): return

  if inst[0] == '(':
    let value = inst.strip(chars = {'(', ')'})
    if isSymbol(value) and not symbols.hasKey(value):
      symbols[value] = linenum

  else:
    linenum += 1

proc processA(inst: string, symbols: var Table, varnum: var int): string =
  var
    value = inst.strip(chars = {'@'})
    num: int

  if isSymbol(value) and not symbols.hasKey(value):
    symbols[value] = varnum
    varnum += 1

  num   = if isSymbol(value): symbols[value]
          else:               value.parseInt

  return "0" & toBin(num, 15)

proc processComp(comp: string): string =
  var
    a = "0"
    comp2 = comp

  if comp2.find("M") > -1:
     a = "1"
     comp2 = comp2.replace("M", "A")

  let temp = case comp2:
               of "0":   "101010"
               of "1":   "111111"
               of "-1":  "111010"
               of "D":   "001100"
               of "A":   "110000"
               of "!D":  "001101"
               of "!A":  "110001"
               of "-D":  "001111"
               of "-A":  "110011"
               of "D+1": "011111"
               of "A+1": "110111"
               of "D-1": "001110"
               of "A-1": "110010"
               of "D+A": "000010"
               of "D-A": "010011"
               of "A-D": "000111"
               of "D&A": "000000"
               of "D|A": "010101"
               else:     "111111"

  result = a & temp

proc processDest(dest: string): string =
  return case dest:
           of "M":   "001"
           of "D":   "010"
           of "MD":  "011"
           of "A":   "100"
           of "AM":  "101"
           of "AD":  "110"
           of "AMD": "111"
           else:     "000"

proc processJmp(jmp: string): string =
  return case jmp:
           of "JGT": "001"
           of "JEQ": "010"
           of "JGE": "011"
           of "JLT": "100"
           of "JNE": "101"
           of "JLE": "110"
           of "JMP": "111"
           else:     "000"

proc processD(inst: string): string =
  var
    line = inst
    temp = line.split('=')
    dest: string
    comp: string
    jmp:  string

  if temp.len() == 2:
    dest = temp[0]
    line = temp[1]
  else:
    dest = ""

  temp = inst.split(';')
  if temp.len() == 2:
    jmp  = temp[1]
    line = temp[0]
  else:
    jmp  = ""

  comp = line

  return "111" & processComp(comp) & processDest(dest) & processJmp(jmp)

proc parse(line: string, symbols: var Table, varnum: var int): string =
  let inst   = stripLine(line)
  if inst.isNilOrWhitespace(): return ""

  let parsed = if inst[0] == '@':   processA(inst, symbols, varnum)
               elif inst[0] != '(': processD(inst)
               else:                ""

  return parsed

proc assemble(fname: string) =
  # only accept .asm files
  if not fname.endsWith(".asm"):
    echo "File \"" & fname & "\" does not end in .asm! Skipping..."
    return

  let
    code    = readfile fname # get the full file contents into a variable
    newname = fname[0..fname.len-5] & ".hack"
  var
    hack    = ""                       # .hack machine code
    symbols = initTable[string, int]() # initialize the symbol table
    linenum = 0
    varnum  = 16

  # analyze the code for symbols
  initSymbols(symbols)
  for line in splitlines(code):
    parseSymbols(line, symbols, linenum)

  # now parse the code
  for line in splitlines(code):
    let parsed = parse(line, symbols, varnum)
    if parsed != "":
      # write parsed line to .hack var
      hack = if hack == "": parsed
             else:          hack & "\n" & parsed

  # write to .hack file and close it in one instruction
  writeFile(newname, hack)

# main loop, assemble each .asm file passed in
for i in 1..paramCount():
  assemble(paramStr(i))
