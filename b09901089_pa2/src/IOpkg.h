#ifndef IOPKG_H
#define IOPKG_H
#include "module.h"
#include <fstream>
#include <iostream>
class IOpkg : public std::fstream {
public:
  IOpkg() : std::fstream() {}
  ~IOpkg(){};
  IOpkg &operator<<(Block &blk) {
    *this << blk.getName() << " " << blk.getX1() << " " << blk.getY1() << " " << blk.getX2() << " " << blk.getY2() << std::endl;
        return *this;
  }
  //IOpkg &operator>>(string &a) {
  //  cout<<"in operator>>"<<endl;
  //  if (_dir == 0){
  //    _file >> a;
  //    cout<<a<<endl;
  //  }
    
  //  return *this;
  //}
  //IOpkg &operator>>(int &a) {
  //  cout<<"in operator>>"<<endl;
  //  if (_dir == 0){
  //    _file >> a;
  //    cout<<a<<endl;
  //  }

  //  return *this;
  //}
};

#endif