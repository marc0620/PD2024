#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

class Net {
public:
  // constructor and destructor
  Net(string &name) : _name(name), _lockl(false), _lockr(false) {
    _partCount[0] = 0;
    _partCount[1] = 0;
  }
  ~Net() {}

  // basic access methods
  string getName() const { return _name; }
  int getPartCount(int part) const { return _partCount[part]; }
  vector<int> &getCellList() { return _cellList; }

  // set functions
  void setName(const string name) { _name = name; }
  void setPartCount(int part, const int count) { _partCount[part] = count; }

  // modify methods
  void incPartCount(int part) { ++_partCount[part]; }
  void decPartCount(int part) { --_partCount[part]; }
  void addCell(const int cellId) { _cellList.push_back(cellId); }
  void lock(int part) {
    if (part == 0)
      _lockl = true;
    else
      _lockr = true;
  }
  void unlock() {
    _lockr = false;
    _lockl = false;
  }
  bool getLock() { return _lockl && _lockr; }

private:
  bool _lockr;
  bool _lockl;
  int _partCount[2];       // Cell number in partition A(0) and B(1)
  string _name;            // Name of the net
  vector<int> _cellList;   // List of cells the net is connected to
};

#endif   // NET_H
