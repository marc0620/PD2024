#ifndef IOPKG_H
#define IOPKG_H
#include "module.h"
#include <fstream>
#include <iostream>
class IOpkg : public std::fstream {
private:
  /* data */
  std::fstream _file;
  bool _dir;   // 0: input, 1: output

public:
  IOpkg() : std::fstream() {}
  ~IOpkg() {
    if (_file.is_open())
      _file.close();
  }
  void open(const char *filename, bool dir) {
    _dir = dir;
    if (dir == 1) {
      _file.open(filename, std::ios::out);
    } else {
      _file.open(filename, std::ios::in);
    }
    if (!_file) {
      std::cerr << "Cannot open the output file \"" << filename << "\". The program will be terminated..." << std::endl;
      exit(1);
    }
    return;
  }
  IOpkg &operator<<(Block &blk) {
    if (_dir == 1)
      _file << blk.getName() << " " << blk.getX1() << " " << blk.getY1() << " " << blk.getX2() << " " << blk.getY2() << std::endl;
    return *this;
  }
};

#endif