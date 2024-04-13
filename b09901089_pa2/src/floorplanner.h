
#ifndef FLOOR_PLANNER_H
#define FLOOR_PLANNER_H

#include "Btree.h"
#include "IOpkg.h"
#include "module.h"
#include <algorithm>
#include <string>
#include <vector>
class floorplanner {
private:
  /* data */
  vector<Net *> _nets;
  vector<Terminal *> _terminals;
  vector<Block *> _blocks;
  Btree _tree;
  IOpkg _inputNet;
  IOpkg _inputBlock;
  IOpkg _output;
  IOpkg _outputplot;
  double _alpha;
  int _outlineX, _outlineY;
  int _blockNum, _terminalNum, _netNum;

public:
  floorplanner(double alpha, char *inputBlk, char *inputNet, char *output);
  void writeOutput();
  void init();
  void plotresult(string filename, int i);
  void rotateBlock(Block *blk);
  void moveBlock(Block *tar, Block *par);
  void swapBlock(Block *blk1, Block *blk2);
  int eval();
  void pack();
  void clearPos();

  ~floorplanner();
};

#endif