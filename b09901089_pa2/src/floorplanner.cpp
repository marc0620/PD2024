#include "floorplanner.h"
#include <algorithm>
#include <map>
#include <vector>
#include "IOpkg.h"

floorplanner::floorplanner(double alpha, char *inputBlk, char *inputNet, char *output) {
  _alpha = alpha;
  _inputNet.open(inputNet, std::ios::in);
  _inputBlock.open(inputBlk, std::ios::in);
  _output.open(output, std::ios::out);
}
void floorplanner::init() {
  string temp,temp1, temp2,temp3;
  map<string, int> termname2id, blockname2id;

  _inputBlock >> temp1 >> _outlineX >> _outlineY;
  _inputBlock >> temp2 >> _blockNum;
  _inputBlock >> temp3 >> _terminalNum;
  _blocks.reserve(_blockNum);
  _terminals.reserve(_terminalNum);
  _nets.reserve(_terminalNum);
  for (int i = 0; i < _blockNum; i++) {
    int w, h;
    string name;
    _inputBlock >> name >> w >> h;
    _blocks.push_back(new Block(name, w, h));
    blockname2id[name] = i;
  }
  for (int i = 0; i < _terminalNum; i++) {
    string name;
    int x, y;
    _inputBlock >> name >> x >> y;
    _terminals.push_back(new Terminal(name, x, y));
    termname2id[name] = i;
  }
  _inputNet >> temp >> _netNum;
  for (int i = 0; i < _netNum; i++) {
    int ntm;
    _inputNet >> temp >> ntm;
    Net *net = new Net();
    for (int j = 0; j < ntm; j++) {
      string name;
      _inputNet >> name;
      if (termname2id.find(name) != termname2id.end())
        net->addTerm(_terminals[termname2id[name]]);
      else
        net->addTerm(_blocks[blockname2id[name]]);
    }
  }
  sort(_blocks.begin(), _blocks.end());
  for (int i = 0; i < _blockNum; i++) {
    cout<< (*_blocks[i]).getName()<<" " << (*_blocks[i]).getWidth()<<" "<<(*_blocks[i]).getHeight()<<endl;
  }
  //_tree.setRoot(new BNode(_blocks[0]));
  //_blocks[0]->setPos(0, 0, _blocks[0]->getWidth(), _blocks[0]->getHeight());
  //_blocks[0]->setNode(_tree.getRoot());
  //Block::setMaxX(_blocks[0]->getWidth());
  //Block::setMaxY(_blocks[0]->getHeight());
  //BNode *home = _tree.getRoot();
  //BNode *cur = home;
  //_tree.updateContour(cur, 0);
  //for (int i = 1; i < _blockNum; i++) {
  //  BNode *node = new BNode(_blocks[i]);
  //  if (cur->getBlk()->getX2() + _blocks[i]->getWidth() >= _outlineX) {
  //    // didn't fit outline x
  //    cur = home;
  //    cur->setRight(node);
  //    _blocks[i]->setX(cur->getBlk()->getX1(), cur->getBlk()->getX1() + _blocks[i]->getWidth());
  //  } else {
  //    // fit outline x
  //    cur->setLeft(node);
  //    _blocks[i]->setX(cur->getBlk()->getX2(), cur->getBlk()->getX2() + _blocks[i]->getWidth());
  //  }
  //  int y = _tree.getY(cur->getBlk()->getX1(), cur->getBlk()->getX2());
  //  _blocks[i]->setY(y, y + _blocks[i]->getHeight());
  //  _blocks[i]->setNode(node);
  //  _tree.updateContour(node, cur->getBlk()->getX2());
  //  cur = node;
  //  if (cur->getBlk()->getX2() > Block::getMaxX())
  //    Block::setMaxX(cur->getBlk()->getX2());
  //  if (cur->getBlk()->getY2() > Block::getMaxY())
  //    Block::setMaxY(cur->getBlk()->getY2());
  //}
}
floorplanner::~floorplanner() {}

void floorplanner::writeOutput() {
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    _output << *(*it);
  }
}