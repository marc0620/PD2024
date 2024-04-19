#include "floorplanner.h"
#include "module.h"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <stdlib.h>
#include <string>
#include <vector>
extern std::fstream &operator<<(std::fstream &fs, Block &blk);

floorplanner::floorplanner(double alpha, char *inputBlk, char *inputNet, char *output) : _alpha(alpha) {
  _alpha = alpha;
  _inputNet.open(inputNet, std::ios::in);
  if (!_inputNet.is_open())
    cout << "Error: cannot open file " << inputNet << endl;
  _inputBlock.open(inputBlk, std::ios::in);
  if (!_inputBlock.is_open())
    cout << "Error: cannot open file " << inputBlk << endl;
  _output.open(output, std::ios::out);
  if (!_output.is_open())
    cout << "Error: cannot open file " << output << endl;
  srand(901089);
}

void floorplanner::revert() {
  _leaves.clear();
  for (auto blk : _blocks) {
    blk->getNode()->revert();
    if (blk->getNode()->getParent() == nullptr) {
      _tree.setRoot(blk->getNode());
    }
    if (blk->getNode()->getLeft() == nullptr && blk->getNode()->getRight() == nullptr) {
      _leaves[blk->getid()] = blk->getNode();
    }
  }
  return;
}

bool floorplanner::checkbest() {
  bool best = false;
  if (_verbose)
    cout << "curcost: " << _curcost << " bestcost: " << _bestcost << endl;
  if (_curcost < _bestcost) {
    best = true;
    _bestcost = _curcost;
    for (auto blk : _blocks) {
      blk->getNode()->setBest();
    }
  }
  return best;
}

bool floorplanner::accept(double cost) {
  double delta = cost - _curcost;
  double prob = exp(-delta / _temp);
  double rv = double(rand()) / RAND_MAX;

  // cout << "curcost " << _curcost << " cost " << cost << " accept prob: " << prob << " rv " << rv << endl;
  if (rv < prob) {
    return true;
  } else {
    return false;
  }
}
void floorplanner::perturb(double r, double m, bool SAmode) {
  double rv = double(rand()) / RAND_MAX;
  // cout << "rv " << rv << " ";
  if (_verbose)
    cout << "p" << _time << ": ";
  if (rv < r) {
    // rotate

    // if (_verbose)
    // cout << "rotate \n";
    int idx = rand() % _blockNum;
    if (_verbose)
      cout << "idx: " << idx << endl;
    rotateBlock(_blocks[idx]);
    pack();
    double cost = eval();
    if (_verbose)
      cout << cost << endl;
    if (accept(cost) || !SAmode) {
      _rnum++;
      _curcost = cost;
      checkbest();
    } else {
      rotateBlock(_blocks[idx]);
      pack();
    }
  } else if (rv < r + m) {
    // move

    // if (_verbose)
    // cout << "move \n";
    int tar = rand() % _leaves.size();
    int par = rand() % _blockNum;
    map<int, BNode *>::iterator it;
    int tarid;
    for (int i = 0; i < _blockNum; i++) {
      it = _leaves.begin();
      std::advance(it, tar);
      tarid = it->first;
      if (tarid != par && (_blocks[par]->getNode()->getLeft() == nullptr || _blocks[par]->getNode()->getRight() == nullptr))
        break;
      par = rand() % _blockNum;
    }
    if (tarid == par || (_blocks[par]->getNode()->getLeft() != nullptr && _blocks[par]->getNode()->getRight() != nullptr))
      return;
    if (_verbose)
      cout << "tar: " << it->second->getBlk()->getid() << " par: " << par << endl;
    BNode *target = (*it).second;
    bool origleft = target->getParent()->getLeft() == target;
    BNode *origpar = target->getParent();
    BNode *parent = _blocks[par]->getNode();
    bool left;
    if (parent->getLeft() != nullptr) {
      left = false;
    } else if (parent->getRight() != nullptr) {
      left = true;
    } else {
      left = (rand() % 3 > 0);
    }
    moveNode(target, parent, left);
    pack();
    double cost = eval();
    if (_verbose)
      cout << cost << endl;
    if (accept(cost) || !SAmode) {
      _mnum++;
      _curcost = cost;
      checkbest();
    } else {
      moveNode(target, origpar, origleft);
      pack();
    }
  } else {
    // swap
    // if (_verbose)
    // cout << "swap \n";
    int n1 = (rand() % (_blockNum));
    int n2 = (rand() % (_blockNum));
    for (int i = 0; i < _blockNum; i++) {
      if (n1 != n2 && (_blocks[n1]->getNode()->getLeft() == nullptr || _blocks[n1]->getNode()->getRight() == nullptr) &&
          (_blocks[n2]->getNode()->getLeft() == nullptr || _blocks[n2]->getNode()->getRight() == nullptr))
        break;
      n2 = (rand() % _blockNum);
    }
    if (n1 == n2) {
      return;
    }
    if (_verbose)
      cout << "n1: " << n1 << " n2: " << n2 << endl;
    BNode *bn1 = _blocks[n1]->getNode();
    BNode *bn2 = _blocks[n2]->getNode();
    swapNode(bn1, bn2);
    pack();
    double cost = eval();
    if (_verbose)
      cout << cost << endl;
    if (accept(cost) || !SAmode) {
      _snum++;
      _curcost = cost;
      // if (checkbest())
      //   plotresult("p" + to_string(_time) + ".svg", _blockNum - 1);
    } else {
      swapNode(bn1, bn2);
      pack();
    }
  }
}

void floorplanner::clear() {
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    (*it)->setPos(0, 0, 0, 0);
  }
  _tree.clearContour();
  Block::setMaxX(0);
  Block::setMaxY(0);
  return;
}

void floorplanner::pack() {
  // pack blocks
  clear();
  BNode *cur = _tree.getRoot();
  if (_verbose)
    cout << cur->getBlk()->getid() << endl;
  cur->getBlk()->setPos(0, 0, cur->getBlk()->getWidth(), cur->getBlk()->getHeight());
  if (cur->getBlk()->getX2() > Block::getMaxX())
    Block::setMaxX(cur->getBlk()->getX2());
  if (cur->getBlk()->getY2() > Block::getMaxY())
    Block::setMaxY(cur->getBlk()->getY2());
  _tree.updateContour(cur);
  for (int i = 1; i < _blockNum; i++) {
    Block *blk;
    if (cur->getLeft() != nullptr) {   // traverse to left
      cur = cur->getLeft();
      // if (_verbose)
      //   cout << "l" << cur->getBlk()->getid() << " ";
      packleft(cur);
    } else if (cur->getRight() != nullptr) {   // traverse to right
      cur = cur->getRight();
      // if (_verbose)
      //   cout << "r" << cur->getBlk()->getid() << " ";
      packright(cur);
    } else {
      while (cur->getParent() != nullptr && (cur->getParent()->getRight() == cur || cur->getParent()->getRight() == nullptr)) {
        cur = cur->getParent();
        // if (_verbose)
        //   cout << "bt" << cur->getBlk()->getid() << " ";
      }
      if (cur->getParent() == nullptr) {
        // if (_verbose)
        //   cout << "end: blocknum: " << _blockNum << " i: " << i << "id " << cur->getBlk()->getid() << endl;
        return;
      }
      // else: traverse to right(same as above)
      // if (_verbose)
      //  cout << "bt" << cur->getParent()->getBlk()->getid() << " " << endl;
      cur = cur->getParent()->getRight();
      // if (_verbose)
      //   cout << "r" << cur->getBlk()->getid() << " ";
      packright(cur);
    }
  }
  if (_verbose)
    cout << endl;
}
void floorplanner::packleft(BNode *cur) {
  Block *blk = cur->getBlk();
  blk->setX(cur->getParent()->getBlk()->getX2(), cur->getParent()->getBlk()->getX2() + blk->getWidth());
  int y = _tree.getY(blk->getX1(), blk->getX2());
  blk->setY(y, y + blk->getHeight());
  _tree.updateContour(cur);
  if (blk->getX2() > Block::getMaxX())
    Block::setMaxX(blk->getX2());
  if (blk->getY2() > Block::getMaxY())
    Block::setMaxY(blk->getY2());
  return;
}
void floorplanner::packright(BNode *cur) {
  Block *blk = cur->getBlk();
  int x = cur->getParent()->getBlk()->getX1();
  blk->setX(x, x + blk->getWidth());
  int y = _tree.getY(blk->getX1(), blk->getX2());
  blk->setY(y, y + blk->getHeight());
  _tree.updateContour(cur);
  if (blk->getX2() > Block::getMaxX())
    Block::setMaxX(blk->getX2());
  if (blk->getY2() > Block::getMaxY())
    Block::setMaxY(blk->getY2());
  return;
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
    string name, garbage;
    int x, y;
    _inputBlock >> name >> garbage >> x >> y;
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
      else if (blockname2id.find(name) != blockname2id.end())
        net->addTerm(_blocks[blockname2id[name]]);
      else {
        cout << "error: " << name << endl;
        exit(1);
      }
    }
    _nets.push_back(net);
  }
  if(_initsort)
    sort(_blocks.begin(), _blocks.end(), [](Block *a, Block *b) { return *a > *b; });
  if(_initrotate==1){
    for (int i = 0; i < _blockNum; i++) {
      if (_blocks[i]->getWidth() > _blocks[i]->getHeight())
        _blocks[i]->Rotate();
    }
  }else if(_initrotate==2){
    for (int i = 0; i < _blockNum; i++) {
      if (_blocks[i]->getWidth() < _blocks[i]->getHeight())
        _blocks[i]->Rotate();
    }
  }

  for (int i = 0; i < _blockNum; i++) {
    _blocks[i]->setid(i);
  }
  _tree.setRoot(new BNode(_blocks[0]));
  _blocks[0]->setPos(0, 0, _blocks[0]->getWidth(), _blocks[0]->getHeight());
  _blocks[0]->setNode(_tree.getRoot());
  Block::setMaxX(_blocks[0]->getWidth());
  Block::setMaxY(_blocks[0]->getHeight());
  BNode *home = _tree.getRoot();
  BNode *cur = home;
  _tree.updateContour(cur);
  if (_initmethod == 0) {
    for (int i = 1; i < _blockNum; i++) {
      BNode *node = new BNode(_blocks[i]);
      if (cur->getBlk()->getX2() + _blocks[i]->getWidth() >= _outlineX) {
        // didn't fit outline x
        cur = home;
        cur->setRight(node);
        node->setParent(cur);
        home = node;
        packright(node);
      } else {
        // fit outline x
        cur->setLeft(node);
        node->setParent(cur);
        packleft(node);
      }
      cur = node;
      _blocks[i]->setNode(node);
      // for (map<int, CSeg *>::iterator it = _tree.getHContour().begin(); it != _tree.getHContour().end(); it++) {
      //   cout << " " << it->second->getX1() << " " << it->second->getX2() << " " << it->second->getY() << endl;
      // }
      // cout << endl;
    }
  } else if (_initmethod == 1) {
    for (int i = 1; i < _blockNum; i++) {
      BNode *node = new BNode(_blocks[i]);
      _blocks[i]->setNode(node);
      node->setParent(_blocks[(i - 1) / 2]->getNode());
      if (i % 2 == 1)
        _blocks[(i - 1) / 2]->getNode()->setLeft(node);
      else
        _blocks[(i - 1) / 2]->getNode()->setRight(node);
    }
    pack();
    eval();
    plotresult("init.svg", _blockNum - 1);
  }

  for (auto blk : _blocks) {
    if (blk->getNode()->getLeft() == nullptr && blk->getNode()->getRight() == nullptr) {
      _leaves[blk->getid()] = blk->getNode();
    }
  }
  _curcost = eval(true);
  checkbest();
  cout << "init done" << endl;
  // cout << "bestcost: " << _bestcost << endl;
}

double floorplanner::eval(bool init) {
  int costNet = 0;
  for (vector<Net *>::iterator it = _nets.begin(); it != _nets.end(); it++) {
    costNet += (*it)->calcHPWL();
  }

  int costarea = Block::getMaxX() * Block::getMaxY();
  if (init) {
    _avgarea = costarea;
    _avgnet = costNet;
  }
  double cost = costNet * (1 - _alpha) / _avgnet + (_alpha)*costarea / _avgarea;
  cost += std::max(int(Block::getMaxX() - _outlineX), 0) * _OOB;
  cost += std::max(int(Block::getMaxY() - _outlineY), 0) * _OOB;
  _realcost = double(costNet * (1 - _alpha) + (_alpha)*costarea);
  _wirelength = costNet;
  if(_time<1000){
    _avgarea = _marate * _avgarea + (1 - _marate) * costarea;
    _avgnet = _marate * _avgnet + (1 - _marate) * costNet;
  }
  // cout << "costNet: " << costNet / _avgnet << " costarea: " << costarea / _avgarea << " cost " << cost << endl;
  return cost;
}

void floorplanner::swapNode(BNode *n1, BNode *n2) {
  if (_tree.getRoot() == n1)
    _tree.setRoot(n2);
  else if (_tree.getRoot() == n2)
    _tree.setRoot(n1);
  BNode *temp = n1->getParent();
  if (temp != nullptr) {
    if (temp->getLeft() == n1) {
      temp->setLeft(n2);
    } else {
      temp->setRight(n2);
    }
  }
  BNode *temp2 = n2->getParent();
  if (temp2 != nullptr) {
    if (temp2->getLeft() == n2) {
      temp2->setLeft(n1);
    } else {
      temp2->setRight(n1);
    }
  }
  n1->setParent(n2->getParent());
  n2->setParent(temp);
  temp = n1->getLeft();
  if (temp != nullptr)
    temp->setParent(n2);
  temp2 = n2->getLeft();
  if (temp2 != nullptr)
    temp2->setParent(n1);
  n1->setLeft(n2->getLeft());
  n2->setLeft(temp);
  temp = n1->getRight();
  if (temp != nullptr)
    temp->setParent(n2);
  temp2 = n2->getRight();
  if (temp2 != nullptr)
    temp2->setParent(n1);
  n1->setRight(n2->getRight());
  n2->setRight(temp);

  bool n1leaf = (_leaves[n1->getBlk()->getid()] != nullptr);
  bool n2leaf = (_leaves[n2->getBlk()->getid()] != nullptr);

  if (n1leaf) {
    _leaves[n2->getBlk()->getid()] = n2;
  } else {
    _leaves.erase(n2->getBlk()->getid());
  }
  if (n2leaf) {
    _leaves[n1->getBlk()->getid()] = n1;
  } else {
    _leaves.erase(n1->getBlk()->getid());
  }

  return;
}

void floorplanner::moveNode(BNode *tar, BNode *par, bool left) {
  if (tar->getParent()->getLeft() == tar) {
    tar->getParent()->setLeft(nullptr);
    if (tar->getParent()->getRight() == nullptr)
      _leaves[tar->getParent()->getBlk()->getid()] = tar->getParent();
  } else {
    tar->getParent()->setRight(nullptr);
    if (tar->getParent()->getLeft() == nullptr)
      _leaves[tar->getParent()->getBlk()->getid()] = tar->getParent();
  }
  if (left) {
    if (par->getLeft() != nullptr) {
      exit(1);
    }
    par->setLeft(tar);
  } else {
    if (par->getRight() != nullptr) {
      exit(1);
    }
    par->setRight(tar);
  }
  if (_leaves.find(par->getBlk()->getid()) != _leaves.end())
    _leaves.erase(par->getBlk()->getid());
  if (_leaves.find(tar->getParent()->getBlk()->getid()) != _leaves.end() && tar->getParent()->getLeft() == nullptr && tar->getParent()->getRight() == nullptr)
    _leaves[tar->getParent()->getBlk()->getid()] = tar->getParent();

  tar->setParent(par);
  return;
}

void floorplanner::rotateBlock(Block *blk) {
  blk->Rotate();
  return;
}

void floorplanner::SA() {
  while (_time <= _maxiter) {
    double r, m;
    // schedule
    if(_temp<100){
      _lambda=_lambda1;
    }
    if (_greedygood) {
      r = 0;
      m = 0;
    } else {
      if (_temp > _first_temp * 0.001)
        r = 0.3, m = 0.4;
      else if (_temp > _first_temp * 0.00001)
        r = 0.4, m = 0.4;
      else
        r = 0.6, m = 0.2;
    }
    perturb(r, m, true);
    if (_verbose)
      plotresult("p" + to_string(_time) + ".svg", _blockNum - 1);
    _time++;
    _temp *= _lambda;
    if (_time % _checkpoint == 0){
      if(_previouscost==_curcost && _bestcost==_curcost)
        return;
      _previouscost=_curcost;
    }
    if(_time%1000==0){
      revert();
    }
  }
  return;
}

void floorplanner::greedy() {
  double totalarea=0;
  for (auto blk : _blocks) {
    totalarea+=blk->getArea();
  }
  for(int i=0;i<_blockNum;i++){
    if(_blocks[i]->getWidth()<_blocks[i]->getHeight())
      _blocks[i]->Rotate();
  }
  for(int i=0;i<2;i++){
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
      if (cur->getBlk()->getX2() + _blocks[i]->getWidth() >= (totalarea/_outlineY)*1.1) {
        // didn't fit outline x
        cur = home;
        cur->setRight(node);
        node->setParent(cur);
        home = node;
        packright(node);
      } else {
        // fit outline x
        cur->setLeft(node);
        node->setParent(cur);
        packleft(node);
      }
      cur = node;
      _blocks[i]->setNode(node);
    }
    _curcost = eval();
    if(Block::getMaxX()*Block::getMaxY()<1.2*totalarea){
      _greedygood=true;
      _first_temp=10;
      cout<<"greedy good"<<endl;
    }else{
      _greedygood=false;
      for(int i=0;i<_blockNum;i++){
        delete _blocks[i]->getNode();
        _blocks[i]->setNode(nullptr);
        if(i==0)
          _blocks[i]->Rotate();
        else
          _blocks[i]->setRotate(0);
      }
      _tree.clearContour();
      _tree.setRoot(nullptr);
    }
    plotresult("greedy"+to_string(i)+".svg", _blockNum - 1);
  }
  
}

/*
IO functions
*/
void floorplanner::writeOutput(double runtime) {
  _output << _realcost << "\n";
  _output << _wirelength << "\n";
  _output << Block::getMaxX() * Block::getMaxY() << "\n";
  _output << Block::getMaxX() << " " << Block::getMaxY() << "\n";
  _output << runtime << "\n";
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    _output << *(*it);
  }
}
void floorplanner::plotresult(string filename, int i) {
  fstream outputplot;
  outputplot.open(filename, std::ios::out);
  outputplot << "<html>\n<body>" << endl;
  outputplot << "<svg width=\"" << _outlineX + 1000 << "\" height=\"" << _outlineY + 1000 << "\" style= \" background-color:white\">" << endl;
  int count = 0;
  for (map<int, CSeg *>::iterator it = _tree.getHContour().begin(); it != _tree.getHContour().end(); it++) {
    outputplot << "<line x1=\"" << it->first << "\" y1=\"" << it->second->getY() << "\" x2=\"" << it->second->getX2() << "\" y2=\"" << it->second->getY() << "\" stroke=\"white\" stroke-width=\"1\" />"
               << endl;
  }
  for (vector<Block *>::iterator it = _blocks.begin(); it != _blocks.end(); it++) {
    if ((*it) == nullptr)
      exit(1);
    if (_leaves.find((*it)->getid()) != _leaves.end())
      outputplot << "<rect x=\"" << (*it)->getX1() / _scale << "\" y=\"" << (*it)->getY1() / _scale << "\" width=\"" << (*it)->getWidth() / _scale << "\" height=\"" << (*it)->getHeight() / _scale
                 << "\" fill=\"rgba(" << 256 << "," << 256 << "," << 0 << "," << 0.4 << ")\" stroke = \"black\" stroke-opacity=\"1\" stroke-width=\"1\" />" << endl;
    else {
      outputplot << "<rect x=\"" << (*it)->getX1() / _scale << "\" y=\"" << (*it)->getY1() / _scale << "\" width=\"" << (*it)->getWidth() / _scale << "\" height=\"" << (*it)->getHeight() / _scale
                 << "\" fill=\"rgba(" << 256 << "," << 0 << "," << 0 << "," << 0.4 << ")\" stroke = \"black\" stroke-opacity=\"1\" stroke-width=\"1\" />" << endl;
    }
    // print i to rect
    outputplot << "<text x=\"" << (*it)->getX1() / _scale + (*it)->getWidth() / _scale / 2 << "\" y=\"" << (*it)->getY1() / _scale + (*it)->getHeight() / _scale / 2
               << "\" fill=\"black\" font-size=\"20\" text-anchor=\"middle\" alignment-baseline=\"middle\">" << (*it)->getName() << "</text>" << endl;
    if (count == i)
      break;
    count++;
  }
  // plot tree structure with left right link
  // plot outline
  outputplot << "<rect x=\"0\" y=\"0\" width=\"" << _outlineX / _scale << "\" height=\"" << _outlineY / _scale << "\" fill=\"rgba(" << 0 << "," << 0 << "," << 0 << "," << 0
             << ")\" stroke-opacity=\"1\" stroke-width=\"3\" stroke=\"black\" />" << endl;

  // plot max x and y
  outputplot << "<rect x=\"" << 0 << "\" y=\"" << 0 << "\" width=\"" << Block::getMaxX() / _scale << "\" height=\"" << Block::getMaxY() / _scale << "\" fill=\"rgba(" << 256 << "," << 0 << "," << 0
             << "," << 0 << ")\" stroke-opacity=\"1\" stroke-width=\"3\" stroke=\"cyan\" />" << endl;
  for (auto blk : _blocks) {
    BNode *node = blk->getNode();
    BNode *left = node->getLeft();
    BNode *right = node->getRight();
    if (left != nullptr) {
      outputplot << "<line x1=\"" << (blk->getX1() + blk->getX2()) / 2 / _scale << "\" y1=\"" << (blk->getY1() + blk->getY2()) / 2 / _scale << "\" x2=\""
                 << (left->getBlk()->getX1() + left->getBlk()->getX2()) / 2 / _scale << "\" y2=\"" << (left->getBlk()->getY1() + left->getBlk()->getY2()) / 2 / _scale
                 << "\" stroke=\"blue\" stroke-width=\"3\" />" << endl;
    }
    if (right != nullptr) {
      outputplot << "<line x1=\"" << (blk->getX1() + blk->getX2()) / 2 / _scale << "\" y1=\"" << (blk->getY1() + blk->getY2()) / 2 / _scale << "\" x2=\""
                 << (right->getBlk()->getX1() + right->getBlk()->getX2()) / 2 / _scale << "\" y2=\"" << (right->getBlk()->getY1() + right->getBlk()->getY2()) / 2 / _scale
                 << "\" stroke=\"green\" stroke-width=\"3\" />" << endl;
    }
  }

  outputplot << "</svg>" << endl;
  outputplot << "</body>\n</html>" << endl;
  outputplot.close();
}
