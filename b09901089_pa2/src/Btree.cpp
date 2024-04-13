#include "Btree.h"
#include <map>
#include <vector>

void Btree::updateContour(BNode *node, int x) {
  int x2 = node->getBlk()->getX2();
  vector<int> tar;
  tar.reserve(5);
  vector<int> type;
  type.reserve(5);
  for (std::map<int, CSeg *>::iterator it = _hContour.begin(); it != _hContour.end(); it++) {
    if (it->first > x2) {
      break;
    } else if (x <= it->first && it->first <= x2 && x2 < it->second->getX2()) {
      tar.push_back(it->first);
      type.push_back(0);
      break;
    } else if (it->first < x && x2 < it->second->getX2()) {
      tar.push_back(it->first);
      type.push_back(1);
      break;
    } else if (x <= it->first && it->second->getX2() <= x2) {
      tar.push_back(it->first);
      type.push_back(2);
    } else if (it->first < x && x <= it->second->getX2() && it->second->getX2() <= x2) {
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
      _hContour[x2 + 1] = temp;
      temp->setX1(x2 + 1);
    } else if (tp == 1) {
      int tarx2 = _hContour[t]->getX2();
      _hContour[t]->setX2(x - 1);
      _hContour[x2 + 1] = new CSeg(x2 + 1, tarx2, node->getBlk()->getY2());
    } else if (tp == 2) {
      CSeg *temp = _hContour[t];
      _hContour.erase(t);
      delete temp;
    } else if (tp == 3) {
      _hContour[t]->setX2(x - 1);
    }
  }
  _hContour[x] = new CSeg(x, x2, node->getBlk()->getY2());
  return;
}

int Btree::getY(int x1, int x2) {
  int y = 0;
  for (std::map<int, CSeg *>::iterator it = _hContour.begin(); it != _hContour.end(); it++) {
    if (it->first > x2) {
      break;
    } else if (x1 <= it->first && it->first <= x2) {
      y = std::max(y, it->second->getY());
    }
  }
  return y;
}