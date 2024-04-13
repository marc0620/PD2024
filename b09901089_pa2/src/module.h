#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>
using namespace std;

class BNode;
class Terminal {
public:
  // constructor and destructor
  Terminal(string &name, size_t x, size_t y) : _name(name), _x1(x), _y1(y), _x2(x), _y2(y) {}
  ~Terminal() {}
  // basic access methods
  const string getName() { return _name; }
  const size_t getX1() { return _x1; }
  const size_t getX2() { return _x2; }
  const size_t getY1() { return _y1; }
  const size_t getY2() { return _y2; }

  // set functions
  void setName(string &name) { _name = name; }
  void setPos(size_t x1, size_t y1, size_t x2, size_t y2) {
    _x1 = x1;
    _y1 = y1;
    _x2 = x2;
    _y2 = y2;
  }

protected:
  string _name;   // module name
  size_t _x1;     // min x coordinate of the terminal
  size_t _y1;     // min y coordinate of the terminal
  size_t _x2;     // max x coordinate of the terminal
  size_t _y2;     // max y coordinate of the terminal
};

class Block : public Terminal {
public:
  // constructor and destructor
  Block(string &name, size_t w, size_t h) : Terminal(name, 0, 0), _w(w), _h(h), _rotate(false) {}
  ~Block() {}

  // basic access methods
  const size_t getWidth() { return _rotate ? _h : _w; }
  const size_t getHeight() { return _rotate ? _w : _h; }
  const size_t getArea() { return _h * _w; }
  static size_t getMaxX() { return _maxX; }
  static size_t getMaxY() { return _maxY; }
  bool getRotate() { return _rotate; }
  BNode *getNode() { return _node; }
  void setX(int x1, int x2) {
    _x1 = x1;
    _x2 = x2;
  }
  void setY(int y1, int y2) {
    _y1 = y1;
    _y2 = y2;
  }

  // set functions
  void setWidth(size_t w) { _w = w; }
  void setHeight(size_t h) { _h = h; }
  void Rotate() { _rotate = !_rotate; }
  void setNode(BNode *node) { _node = node; }
  static void setMaxX(size_t x) { _maxX = x; }
  static void setMaxY(size_t y) { _maxY = y; }
  bool operator<(const Block &blk) const { return _w * _h < blk._w * blk._h; }
  bool operator>(const Block &blk) const { return _w * _h > blk._w * blk._h; }

private:
  size_t _w;             // width of the block
  size_t _h;             // height of the block
  static size_t _maxX;   // maximum x coordinate for all blocks
  static size_t _maxY;   // maximum y coordinate for all blocks
  bool _rotate;          // whether the block is rotated
  BNode *_node;          // pointer to the node in the binary tree
};
class Net {
public:
  // constructor and destructor
  Net() {}
  ~Net() {}

  // basic access methods
  const vector<Terminal *> getTermList() { return _termList; }

  // modify methods
  void addTerm(Terminal *term) { _termList.push_back(term); }

  // other member functions
  double calcHPWL();

private:
  vector<Terminal *> _termList;   // list of terminals the net is connected to
};

#endif   // MODULE_H
