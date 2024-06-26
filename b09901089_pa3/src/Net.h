//////////////////  WARNING /////////////////////////
// DO NOT MODIFY THIS FILE, THIS IS THE HEADER FILE
// FOR THE PRECOMPILED LIBRARY.
// IF YOU WANT TO MODIFY THIS FILE, PLEASE ENSURE 
// YOU UNDERSTAND WHAT YOU ARE DOING.
/////////////////////////////////////////////////////

#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

#include "Pin.h"

class Net {
public:
    Net() {}

    /////////////////////////////////////////////
    // get (for pins of this net)
    /////////////////////////////////////////////
    unsigned numPins() {return _pPins.size();}
    Pin& pin(unsigned index) {return *_pPins[index];} // index: 0 ~ (numPins-1), not Pin id

    /////////////////////////////////////////////
    // set (for pins of this net)
    /////////////////////////////////////////////
    void setNumPins(unsigned numPins) {_pPins.resize(numPins);}
    void addPin(Pin *pPin) {_pPins.push_back(pPin);}
    void clearPins() {_pPins.clear();}

private:
    // pins of the module
    vector<Pin *> _pPins;
};

#endif // NET_H
