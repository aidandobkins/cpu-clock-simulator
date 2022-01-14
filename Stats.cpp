/******************************
 * Submitted by: Aidan Dobkins - add136
 * CS 3339 - Spring 2021, Texas State University
 * Project 3 Pipelining
 * Copyright 2021, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ******************************/
 
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  exe1 = 0;
  exe2 = 0;
  mem1 = 0;
  mem2 = 0;
  hazards = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultStage[i] = -1;
  }

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
  }
}

void Stats::clock() {
  cycles++;

  // advance all pipeline flip-flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
    resultStage[i] = resultStage[i-1];
  }
  // inject a NOP in pipestage IF1
  resultReg[IF1] = -1;
  resultStage[IF1] = -1;
}

void Stats::registerSrc(int r, int needed) {
  int amtBubbles = 0;
  for(int i = EXE1; i < WB; i++)
  {
    if(r == resultReg[i])
    {
      hazards++;
      switch(i)
      {
        case EXE1: 
          exe1++;
          break;
        case EXE2: 
          exe2++;
          break;
        case MEM1: 
          mem1++;
          break;
        case MEM2: 
          mem2++;
          break;
      }
      amtBubbles = (resultStage[i] - i) - (needed - i) - 1; 
      break;
    }
  }
  for(int i = 0; i < amtBubbles; i++)
    bubble();
}

void Stats::registerDest(int r, int avail) {
  resultReg[ID] = r;
  resultStage[ID] = avail;
}

void Stats::delay()
{
  hazards++;
  bubble();
}

void Stats::flush(int count) { // count == how many ops to flush
  flushes += count;
  cycles += count;
  for(int j = 0; j < count; j++)
    for (int i = 0; i < PIPESTAGES; i++)
      resultReg[i] = -1;
}

void Stats::bubble() {
  bubbles++;
  clock();
}

void Stats::stall(int count)
{
  for(int i = 0; i < count; i++)
    clock();
}

void Stats::showPipe() {
  // this method is to assist testing and debug, please do not delete or edit
  // you are welcome to use it but remove any debug outputs before you submit
  cout << "              IF1  IF2 *ID* EXE1 EXE2 MEM1 MEM2 WB         #C      #B      #F" << endl; 
  cout << "  resultReg ";
  for(int i = 0; i < PIPESTAGES; i++) {
    cout << "  " << dec << setw(2) << resultReg[i] << " ";
  }
  cout << "   " << setw(7) << cycles << " " << setw(7) << bubbles << " " << setw(7) << flushes;
  cout << endl;
}
