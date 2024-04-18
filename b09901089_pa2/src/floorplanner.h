
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
  double _OOB = 1;
  double _bestcost = INT_MAX, _curcost = INT_MAX;
  int _first_temp = 100000;
  double _temp = _first_temp;
  int _time = 0;
  double _lambda = 0.995;
  double _lambda1 = 0.9999;
  double _lambda2 = 0.9999;
  double _marate = 0.99;
  int _maxiter = 300000;
  bool _verbose = false;
  int _checkpoint=10000; 
  double _previouscost = 0;
  double _avgarea = 0, _avgnet = 0;
  double _realcost = 0;
  int _wirelength = 0;
  int _badcount = 0;
  int _rnum = 0, _mnum = 0, _snum = 0;
  int _initmethod = 1;   // 0: row by row 1: complete tree
  bool _initsort = 1;
  int _initrotate = 0; //0: no rotate 1: vertical 2: horizontal
  bool _greedygood = 0;
  int _scale=2;


public:
  floorplanner(double alpha, char *inputBlk, char *inputNet, char *output);
  int getrnum() { return _rnum; }
  int getmnum() { return _mnum; }
  int getsnum() { return _snum; }
  void writeOutput(double runtime);
  void init();
  void plotresult(string filename, int i);
  void rotateBlock(Block *blk);
  void moveNode(BNode *tar, BNode *par, bool left);
  void swapNode(BNode *blk1, BNode *blk2);
  double eval(bool init = false);
  void pack();
  void clear();
  void packleft(BNode *cur);
  void packright(BNode *cur);
  int getBlkNum() { return _blockNum; }
  void SA();
  void perturb(double r, double m, bool SAmode);
  bool checkbest();
  void revert();
  bool accept(double cost);
  void greedy();
  ~floorplanner(){};
};

#endif