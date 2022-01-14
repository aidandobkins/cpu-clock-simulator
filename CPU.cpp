/******************************
 * Submitted by: Aidan Dobkins - add136
 * CS 3339 - Spring 2021, Texas State University
 * Project 1 Disassembler
 * Copyright 2021, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ******************************/

#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};



CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();
    mem();
    writeback();

    stats.clock();

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION 
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t simm;         // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

  opcode = instr >> 26; //shift to first 6 bits
  rs = (instr >> 21) & 0x1f; //shift, then and mask all but 5
  rt = (instr >> 16) & 0x1f; //shift, then and mask all but 5
  rd = (instr >> 11) & 0x1f; //shift, then and mask all but 5
  shamt = (instr >> 6) & 0x1f; //shift, then and mask all but 5
  funct = instr & 0x3f; //mask all but 6 bits
  uimm = instr & 0xffff; //mask
  simm = instr & 0xffff; //mask
  addr = instr & 0x3ffffff; //mask

  if ((simm >> 15) == 1) //if signed, then remove padding
  {
    simm = simm - 0x10000;
  }

  // Hint: you probably want to give all the control signals some "safe"
  // default value here, and then override their values as necessary in each
  // case statement below!

  opIsLoad = false;
  opIsStore = false;
  opIsMultDiv = false;
  aluOp = ADD;
  writeDest = false;
  destReg = REG_ZERO;
  aluSrc1 = 0; 
  aluSrc2 = 0;
  storeData = 0;

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   aluOp = SHF_L; //shift left, following alu.cpp
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   stats.registerDest(rd, MEM1);
                   aluSrc2 = shamt;
                   destReg = rd; //write aluout to rd 
                   writeDest = true;
                   break; // use prototype above, not the greensheet
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   aluOp = SHF_R; //shift right, following alu.cpp
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   stats.registerDest(rd, MEM1);
                   aluSrc2 = shamt;
                   destReg = rd; //write aluout to rd
                   writeDest = true;
                   break; // use prototype above, not the greensheet
        case 0x08: D(cout << "jr " << regNames[rs]);
                   aluOp = ADD; //pc should pass thru ALU without changes
                   aluSrc1 = pc;
                   aluSrc2 = regFile[REG_ZERO]; // always reads zero
                   stats.registerSrc(rs, ID);
                   pc = regFile[rs];
                   stats.delay(); //branch delay
                   stats.flush(2);
                   break;
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                   writeDest = true;
                   destReg = rd; //write aluout to rd
                   stats.registerSrc(REG_HILO, EXE1);
                   stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = hi; //add hi to 0, leaving with just hi in aluout
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]);
                   writeDest = true;
                   destReg = rd; //write aluout to rd
                   stats.registerSrc(REG_HILO, EXE1);
                   stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = lo; //add lo to 0, leaving with just lo in aluout
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = MUL; //easy follow of alu.cpp
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt];
                   stats.registerSrc(rt, EXE1);
                   stats.registerDest(REG_HILO, WB);
                   opIsMultDiv = true;
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = DIV; //easy follow of alu.cpp
                   opIsMultDiv = true;
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt];
                   stats.registerSrc(rt, EXE1);
                   stats.registerDest(REG_HILO, WB);
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = ADD; //easy follow of alu.cpp
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt];
                   stats.registerSrc(rt, EXE1);
                   stats.registerDest(rd, MEM1);
                   destReg = rd; //write aluout to rd
                   writeDest = true;
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = ADD; //easy follow of alu.cpp, but negative
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   aluSrc2 = (0 - regFile[rt]);
                   stats.registerSrc(rt, EXE1);
                   stats.registerDest(rd, MEM1);
                   destReg = rd; //write aluout to rd
                   writeDest = true;
                   break; //hint: subtract is the same as adding a negative
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = CMP_LT;
                   destReg = rd; //write aluout to rd
                   writeDest = true;
                   aluSrc1 = regFile[rs];
                   stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt];
                   stats.registerSrc(rt, EXE1);
                   stats.registerDest(rd, MEM1);
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               aluOp = ADD; //pc should pass thru ALU without changes
               aluSrc1 = pc; //same as jal, but doesnt link to ra
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               stats.registerSrc(REG_ZERO, EXE1);
               pc = (pc & 0xf0000000) | addr << 2;
               stats.flush(2);
               break;
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
               aluOp = ADD; //pc should pass thru ALU without changes
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               stats.registerSrc(REG_ZERO, EXE1);
               stats.registerDest(REG_RA, EXE1);
               pc = (pc & 0xf0000000) | addr << 2;
               stats.delay(); //branch delay
               stats.flush(2);
               break;
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               stats.registerSrc(rs, ID);
               stats.registerSrc(rt, ID);
               if(regFile[rs] == regFile[rt]) //simple if statement for detecting, then use of project1 bit shifted pc
               {
                stats.countTaken();
                pc = pc + (simm << 2);
                stats.flush(2);
               }
               stats.countBranch();
               break;  // read the handout carefully, update PC directly here as in jal example
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               stats.registerSrc(rs, ID);
               stats.registerSrc(rt, ID);
               if(regFile[rs] != regFile[rt]) //simple if statement for detecting, then use of project1 bit shifted pc
               {
                stats.countTaken();
                pc = pc + (simm << 2);
                stats.flush(2);
               }
               stats.countBranch();
               break;  // same comment as beq
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               aluOp = ADD; //easy follow of alu.cpp
               aluSrc1 = regFile[rs];
               stats.registerSrc(rs, EXE1);
               stats.registerDest(rt, MEM1);
               aluSrc2 = simm;
               destReg = rt;
               writeDest = true;
               break;
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
               aluOp = AND; //easy follow of alu.cpp
               aluSrc1 = uimm;
               aluSrc2 = regFile[rs];
               stats.registerSrc(rs, EXE1);
               stats.registerDest(rt, MEM1);
               writeDest = true;
               destReg = rt;
               break;
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               aluOp = SHF_L; //shifts left 16 to shift upper to the location to load
               aluSrc1 = simm;
               aluSrc2 = 16;
               writeDest = true;
               destReg = rt;
               stats.registerDest(rt, MEM1);
               break; //use the ALU to perform necessary op, you may set aluSrc2 = xx directly
    case 0x1a: D(cout << "trap " << hex << addr);
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs];
                           stats.registerSrc(rs, EXE1);
                           break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt];
                           stats.registerDest(rt, MEM1);
                           break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               opIsLoad = true; //easy follow of mem.cpp
               aluOp = ADD;
               aluSrc1 = simm;
               aluSrc2 = regFile[rs];
               stats.registerSrc(rs, EXE1);
               stats.registerDest(rt, WB);
               writeDest = true;
               destReg = rt;
               stats.countMemOp();
               break;  // do not interact with memory here - setup control signals for mem()
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               opIsStore = true; //easy follow of mem.cpp
               aluOp = ADD;
               aluSrc1 = simm;
               aluSrc2 = regFile[rs];
               stats.registerSrc(rs, EXE1);
               stats.registerSrc(rt, MEM1);
               storeData = regFile[rt];
               stats.countMemOp();
               break;  // same comment as lw
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad)
  {
    writeData = dMem.loadWord(aluOut);
    stats.stall(cacheStats.access(aluOut, LOAD));
  }
  else
    writeData = aluOut;

  if(opIsStore)
  {
    dMem.storeWord(storeData, aluOut);
    stats.stall(cacheStats.access(aluOut, STORE));
  }
}

void CPU::writeback() {
  if(writeDest && destReg > 0) // skip when write is to zero_register
    regFile[destReg] = writeData;
  
  if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl << endl;

  cout << "Cycles: " << stats.getCycles() << endl
       << "CPI: " << fixed << setprecision(2) << (double(stats.getCycles()) / double(instructions)) << endl << endl
       << "Bubbles: " << stats.getBubbles() << endl
       << "Flushes: " << stats.getFlushes() << endl
       << "Stalls: " << cacheStats.getStalls() << endl << endl;

  cacheStats.printFinalStats();
}
