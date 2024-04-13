#include "floorplanner.h"
#include "IOpkg.h"
#include "module.h"
#include <algorithm>
#include <map>
#include <vector>
floorplanner::floorplanner(double alpha, char *inputBlk, char *inputNet, char *output) {
  _alpha = alpha;
  _inputNet.open(inputNet, std::ios::in);
  _inputBlock.open(inputBlk, std::ios::in);
  _output.open(output, std::ios::out);
  _outputplot.open("plot.svg", std::ios::out);
}
void floorplanner::clearPos() {
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    (*it)->setPos(0, 0, 0, 0);
  }
}
void floorplanner::pack() {
  // pack blocks
  BNode *cur = _tree.getRoot();
  cur->getBlk()->setPos(0, 0, cur->getBlk()->getWidth(), cur->getBlk()->getHeight());
  _tree.updateContour(cur);
  for (int i = 1; i < _blockNum; i++) {
    Block *blk;
    if (cur->getLeft() != nullptr) {   // traverse to left
      cur = cur->getLeft();
      blk = cur->getBlk();
      blk->setX(cur->getParent()->getBlk()->getX2(), cur->getParent()->getBlk()->getX2() + blk->getWidth());
      int y = _tree.getY(blk->getX1(), blk->getX2());
      blk->setY(y, y + blk->getHeight());
      _tree.updateContour(cur);
    } else if (cur->getRight() != nullptr) {   // traverse to right
      cur = cur->getRight();
      blk = cur->getBlk();
      int x = cur->getParent()->getBlk()->getX1();
      blk->setX(x, x + blk->getWidth());
      int y = _tree.getY(blk->getX1(), blk->getX2());
      blk->setY(y, y + blk->getHeight());
      _tree.updateContour(cur);
    } else {
      while (cur->getParent() != nullptr && (cur->getParent()->getRight() == cur || cur->getParent()->getRight() == nullptr)) {
        cur = cur->getParent();
      }
      if (cur->getParent() == nullptr) {
        cout << "blocknum: " << _blockNum << " i: " << i << endl;
        return;
      }
      // else: traverse to right(same as above)
      cur = cur->getParent()->getRight();
      blk = cur->getBlk();
      int x = cur->getParent()->getBlk()->getX1();
      blk->setX(x, x + blk->getWidth());
      int y = _tree.getY(blk->getX1(), blk->getX2());
      blk->setY(y, y + blk->getHeight());
      _tree.updateContour(cur);
    }
    if (blk->getX2() > Block::getMaxX())
      Block::setMaxX(blk->getX2());
    if (blk->getY2() > Block::getMaxY())
      Block::setMaxY(blk->getY2());
  }
}
void floorplanner::init() {
  string temp, temp1, temp2, temp3;
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

  sort(_blocks.begin(), _blocks.end(), [](Block *a, Block *b) { return *a > *b; });
  for (int i = 0; i < _blockNum; i++) {
    if (_blocks[i]->getWidth() < _blocks[i]->getHeight())
      _blocks[i]->Rotate();
  }
  _tree.setRoot(new BNode(_blocks[0]));
  _blocks[0]->setPos(0, 0, _blocks[0]->getWidth(), _blocks[0]->getHeight());
  _blocks[0]->setNode(_tree.getRoot());
  Block::setMaxX(_blocks[0]->getWidth());
  Block::setMaxY(_blocks[0]->getHeight());
  BNode *home = _tree.getRoot();
  BNode *cur = home;
  _tree.updateContour(cur);
  for (int i = 1; i < _blockNum; i++) {
    BNode *node = new BNode(_blocks[i]);
    if (cur->getBlk()->getX2() + _blocks[i]->getWidth() >= _outlineX) {
      // didn't fit outline x
      cur = home;
      cur->setRight(node);
      _blocks[i]->setX(cur->getBlk()->getX1(), cur->getBlk()->getX1() + _blocks[i]->getWidth());
      home = node;
    } else {
      // fit outline x
      cur->setLeft(node);
      _blocks[i]->setX(cur->getBlk()->getX2(), cur->getBlk()->getX2() + _blocks[i]->getWidth());
    }
    int y = _tree.getY(_blocks[i]->getX1(), _blocks[i]->getX2());
    _blocks[i]->setY(y, y + _blocks[i]->getHeight());
    _blocks[i]->setNode(node);
    node->setParent(cur);
    cur = node;
    _tree.updateContour(node);
    if (node->getBlk()->getX2() > Block::getMaxX())
      Block::setMaxX(node->getBlk()->getX2());
    if (node->getBlk()->getY2() > Block::getMaxY())
      Block::setMaxY(node->getBlk()->getY2());

    // for (map<int, CSeg *>::iterator it = _tree.getHContour().begin(); it != _tree.getHContour().end(); it++) {
    //   cout << " " << it->second->getX1() << " " << it->second->getX2() << " " << it->second->getY() << endl;
    // }
    // cout << endl;
  }
  writeOutput();
  plotresult("plot.svg", _blockNum - 1);
}
floorplanner::~floorplanner() {}

void floorplanner::writeOutput() {
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    _output << *(*it);
  }
}

void floorplanner::plotresult(string filename, int i) {
  IOpkg outputplot;
  outputplot.open(filename, std::ios::out);
  outputplot << "<html>\n<body>" << endl;
  outputplot << "<svg width=\"" << _outlineX + 1000 << "\" height=\"" << _outlineY + 1000 << "\" style= \" background-color:white\">" << endl;
  int count = 0;
  for (map<int, CSeg *>::iterator it = _tree.getHContour().begin(); it != _tree.getHContour().end(); it++) {
    outputplot << "<line x1=\"" << it->first << "\" y1=\"" << it->second->getY() << "\" x2=\"" << it->second->getX2() << "\" y2=\"" << it->second->getY() << "\" stroke=\"white\" stroke-width=\"1\" />"
               << endl;
  }
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    outputplot << "<rect x=\"" << (*it)->getX1() << "\" y=\"" << (*it)->getY1() << "\" width=\"" << (*it)->getWidth() << "\" height=\"" << (*it)->getHeight() << "\" fill=\"rgba(" << 256 << "," << 0
               << "," << 0 << "," << 0.4 << ")\" stroke-opacity=\"0\" stroke-width=\"0\" />" << endl;
    // print i to rect
    outputplot << "<text x=\"" << (*it)->getX1() + (*it)->getWidth() / 2 << "\" y=\"" << (*it)->getY1() + (*it)->getHeight() / 2
               << "\" fill=\"black\" font-size=\"20\" text-anchor=\"middle\" alignment-baseline=\"middle\">" << count << "</text>" << endl;
    if (count == i)
      break;
    count++;
  }
  // plot tree structure with left right link
  // plot outline
  outputplot << "<rect x=\"0\" y=\"0\" width=\"" << _outlineX << "\" height=\"" << _outlineY << "\" fill=\"rgba(" << 0 << "," << 0 << "," << 0 << "," << 0
             << ")\" stroke-opacity=\"1\" stroke-width=\"10\" stroke=\"black\" />" << endl;
  outputplot << "</svg>" << endl;
}
