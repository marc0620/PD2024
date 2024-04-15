#include "Btree.h"
#include <iostream>
#include <map>
#include <vector>

void BNode::revert() {
  _parent = _bparent;
  _left = _bleft;
  _right = _bright;
  return;
}

void BNode::setBest() {
  _bparent = _parent;
  _bleft = _left;
  _bright = _right;
  return;
}

void Btree::updateContour(BNode *node) {
  int x1 = node->getBlk()->getX1();
  int x2 = node->getBlk()->getX2();
  vector<int> tar;
  tar.reserve(5);
  vector<int> type;
  type.reserve(5);
  for (std::map<int, CSeg *>::iterator it = _hContour.begin(); it != _hContour.end(); it++) {
    if (it->first > x2) {
      break;
    } else if (x1 <= it->first && it->first < x2 && x2 < it->second->getX2()) {
      tar.push_back(it->first);
      type.push_back(0);
      break;
    } else if (it->first < x1 && x2 < it->second->getX2()) {
      tar.push_back(it->first);
      type.push_back(1);
      break;
    } else if (x1 <= it->first && it->second->getX2() <= x2) {
      tar.push_back(it->first);
      type.push_back(2);
    } else if (it->first < x1 && x1 < it->second->getX2() && it->second->getX2() <= x2) {
      tar.push_back(it->first);
      type.push_back(3);
    }
  }
  while (!tar.empty()) {
    int t = tar.back();
    tar.pop_back();
    int tp = type.back();
    type.pop_back();
    if (tp == 0) {
      CSeg *temp = _hContour[t];
      _hContour.erase(t);
      _hContour[x2] = temp;
      temp->setX1(x2);
    } else if (tp == 1) {
      int tarx2 = _hContour[t]->getX2();
      _hContour[t]->setX2(x1);
      _hContour[x2] = new CSeg(x2, tarx2, node->getBlk()->getY2());
    } else if (tp == 2) {
      CSeg *temp = _hContour[t];
      _hContour.erase(t);
      delete temp;
    } else if (tp == 3) {
      _hContour[t]->setX2(x1);
    }
  }
  _hContour[x1] = new CSeg(x1, x2, node->getBlk()->getY2());
  return;
}

int Btree::getY(int x1, int x2) {
  int y = 0;
  for (std::map<int, CSeg *>::iterator it = _hContour.begin(); it != _hContour.end(); it++) {
    if (x1 <= it->first && it->first < x2) {
      y = std::max(y, it->second->getY());
    } else if (x2 <= it->first) {
      break;
    }
  }
  return y;
}
void Btree::clearContour() {
  for (std::map<int, CSeg *>::iterator it = _hContour.begin(); it != _hContour.end(); it++) {
    delete it->second;
  }
  _hContour.clear();
  return;
}