
#ifndef FLOOR_PLANNER_H
#define FLOOR_PLANNER_H

#include "Btree.h"
#include "module.h"
#include <algorithm>
#include <climits>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
class floorplanner {
private:
  /* data */
  vector<Net *> _nets;
  vector<Terminal *> _terminals;
  vector<Block *> _blocks;
  Btree _tree;
  fstream _inputNet;
  fstream _inputBlock;
  fstream _output;
  double _alpha;
  size_t _outlineX, _outlineY;
  int _blockNum, _terminalNum, _netNum;
  map<int, BNode *> _leaves;
  // modifiable variables
  int _OOB = 1;
  double _bestcost = INT_MAX, _curcost = INT_MAX;
  int _first_temp = 1000000;
  double _temp = _first_temp;
  int _time = 0;
  double _lambda = 0.99;
  bool _verbose = false;
  double _avgarea=0, _avgnet=0;

public:
  floorplanner(double alpha, char *inputBlk, char *inputNet, char *output);
  void writeOutput();
  void init();
  void plotresult(string filename, int i);
  void rotateBlock(Block *blk);
  void moveNode(BNode *tar, BNode *par, bool left);
  void swapNode(BNode *blk1, BNode *blk2);
  double eval(bool init=false);
  void pack();
  void clear();
  void packleft(BNode *cur);
  void packright(BNode *cur);
  int getBlkNum() { return _blockNum; }
  void SA();
  void perturb(double r, double m, bool SAmode);
  bool checkbest();
  void revert();
  bool accept(int cost);
  ~floorplanner(){};
};

#endif