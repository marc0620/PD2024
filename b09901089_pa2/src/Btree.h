#ifndef BTREE_H
#define BTREE_H
#include "module.h"
#include <map>
#include <string>
#include <vector>

class CSeg {
private:
  size_t _x1;
  size_t _x2;
  size_t _y;

public:
  CSeg(int x1, int x2, int y) : _x1(x1), _x2(x2), _y(y) {}
  ~CSeg() {}
  int getX2() { return _x2; }
  int getX1() { return _x1; }
  int getY() { return _y; }
  void setX1(int x1) { _x1 = x1; }
  void setX2(int x2) { _x2 = x2; }
};

class BNode {
private:
  BNode *_left;
  BNode *_right;
  Block *_blk;
  BNode *_parent;

public:
  BNode(Block *blk) : _blk(blk), _left(nullptr), _right(nullptr), _parent(nullptr) {}
  ~BNode() {}
  BNode *getLeft() { return _left; }
  BNode *getRight() { return _right; }
  BNode *getParent() { return _parent; }
  Block *getBlk() { return _blk; }
  void setLeft(BNode *left) { _left = left; }
  void setRight(BNode *right) { _right = right; }
  void setParent(BNode *parent) { _parent = parent; }
  void setBlk(Block *blk) { _blk = blk; }
};

class Btree {
private:
  BNode *_root;
  std::map<int, CSeg *> _hContour;

public:
  Btree() : _root(nullptr) {}
  ~Btree() {}
  void updateContour(BNode *node, int x);
  BNode *getRoot() { return _root; }
  void setRoot(BNode *root) { _root = root; }
  int getY(int x1, int x2);
};

#endif