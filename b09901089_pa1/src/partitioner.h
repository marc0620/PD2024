#ifndef PARTITIONER_H
#define PARTITIONER_H

#include "cell.h"
#include "net.h"
#include <fstream>
#include <map>
#include <vector>
using namespace std;

class Partitioner {
public:
  // constructor and destructor
  Partitioner(fstream &inFile) : _cutSize(0), _netNum(0), _cellNum(0), _maxPinNum(0), _bFactor(0), _accGain(0), _maxAccGain(0), _iterNum(0),_bestCut(-1),_perturbNum(0),_cutRatio(0.005),_perturbRatio(0.5),_maxIter(10),_perturbPeriod(1){
    parseInput(inFile);
    _partSize[0] = 0;
    _partSize[1] = 0;
  }
  ~Partitioner() { clear(); }

  // basic access methods
  int getCutSize() const { return _cutSize; }
  int getNetNum() const { return _netNum; }
  int getCellNum() const { return _cellNum; }
  double getBFactor() const { return _bFactor; }
  int getPartSize(int part) const { return _partSize[part]; }

  // modify method
  void parseInput(fstream &inFile);
  void partition();

  // member functions about reporting
  void printSummary() const;
  void reportNet() const;
  void reportCell() const;
  void writeResult(fstream &outFile);
  void printslist();

private:
  int _cutSize;                 // cut size
  int _partSize[2];             // size (cell number) of partition A(0) and B(1)
  int _netNum;                  // number of nets
  int _cellNum;                 // number of cells
  int _maxPinNum;               // Pmax for building bucket list
  double _bFactor;              // the balance factor to be met
  Node *_maxGainCell;           // pointer to max gain cell
  vector<Net *> _netArray;      // net array of the circuit
  vector<Cell *> _cellArray;    // cell array of the circuit
  map<int, Node *> _bList[2];   // bucket list of partition A(0) and B(1)
  map<int, Node *> _sListtail;
  map<int, Node *> _sList;
  map<string, int> _netName2Id;    // mapping from net name to id
  map<string, int> _cellName2Id;   // mapping from cell name to id
  int _accGain;                    // accumulative gain
  int _maxAccGain;                 // maximum accumulative gain
  int _moveNum;                    // number of cell movements
  int _iterNum;                    // number of iterations
  int _bestMoveNum;                // store best number of movements
  int _unlockNum[2];               // number of unlocked cells
  map<int,int> _affectedCell;

  int _threshold;
  int _bestCut;
  double _cutRatio;
  double _perturbRatio;
  int _perturbNum;
  int _perturbPeriod;
  int _initcutsize;
  double _perturbStep;
  int _maxIter;
  vector<int> _moveStack;   // history of cell movement
  void addBefore(int part, Node *node, int gain);
  void addAfter(int part, Node *node, int gain);
  void remove(Node *node, int gain);
  void move(Node *tar,int mode); // mode 0: initial partition, mode 1: partition process
  void updateGain(Cell *cell);
  bool refine(int mode);
  void initialPartition(int mode);
  void perturb();
  void restore(); 
  int calculateGain(bool unlocknet);
  void setup(bool increase_iter);
  int calculateCutSize();
  // Clean up partitioner
  void clear();
};

#endif   // PARTITIONER_H
