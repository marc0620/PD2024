#include "module.h"
#include <fstream>
#include <iostream>
#include <limits.h>
size_t Block::_maxX = 0;
size_t Block::_maxY = 0;
std::fstream &operator<<(std::fstream &fs, Block &blk) {
  fs << blk.getName() << " " << blk.getX1() << " " << blk.getY1() << " " << blk.getX2() << " " << blk.getY2() << std::endl;
  return fs;
}
double Net::calcHPWL() {
  size_t x1 = INT_MAX, x2 = 0, y1 = INT_MAX, y2 = 0;
  for (auto term : _termList) {
    x1 = std::min(x1, term->getX1());
    x2 = std::max(x2, term->getX1());
    y1 = std::min(y1, term->getY1());
    y2 = std::max(y2, term->getY1());
  }
  return (x2 - x1) + (y2 - y1);
}
