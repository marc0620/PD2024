
#ifndef FLOOR_PLANNER_H
#define FLOOR_PLANNER_H

#include <algorithm>
#include "Btree.h"
#include "IOpkg.h"
#include "module.h"
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
  double _alpha;
  int _outlineX, _outlineY;
  int _blockNum, _terminalNum, _netNum;

public:
  floorplanner(double alpha, char *inputBlk, char *inputNet, char *output);
  void writeOutput();
  void init();
  ~floorplanner();
};

#endif