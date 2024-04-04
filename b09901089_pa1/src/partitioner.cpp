#include "partitioner.h"
#include "cell.h"
#include "net.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

#define MAX_ITER 1000
void Partitioner::parseInput(fstream &inFile) {
  string str;
  // Set balance factor
  inFile >> str;
  _bFactor = stod(str);
  _threshold = _cellNum * (1 - _bFactor) / 2;

  // Set up whole circuit
  while (inFile >> str) {
    if (str == "NET") {
      string netName, cellName, tmpCellName = "";
      inFile >> netName;
      int netId = _netNum;
      _netArray.push_back(new Net(netName));
      _netName2Id[netName] = netId;
      while (inFile >> cellName) {
        if (cellName == ";") {
          tmpCellName = "";
          break;
        } else {
          // a newly seen cell
          if (_cellName2Id.count(cellName) == 0) {
            int cellId = _cellNum;
            _cellArray.push_back(new Cell(cellName, 0, cellId));
            _cellName2Id[cellName] = cellId;
            _cellArray[cellId]->addNet(netId);
            _cellArray[cellId]->incPinNum();
            _netArray[netId]->addCell(cellId);
            ++_cellNum;
            tmpCellName = cellName;
          }
          // an existed cell
          else {
            if (cellName != tmpCellName) {
              assert(_cellName2Id.count(cellName) == 1);
              int cellId = _cellName2Id[cellName];
              _cellArray[cellId]->addNet(netId);
              _cellArray[cellId]->incPinNum();
              _netArray[netId]->addCell(cellId);
              tmpCellName = cellName;
            }
          }
        }
      }
      ++_netNum;
    }
  }
  return;
}
void Partitioner::addBefore(int part, Node *node, int gain) {
  if (_sList[gain] == NULL) {
    _sList[gain] = node;
    return;
  } else {
    node->setNext(_sList[gain]);
    node->setPrev(NULL);
    _sList[gain]->setPrev(node);
    _sList[gain] = node;
    return;
  }
}
void Partitioner::updateGain(Cell *cell) {
  for (vector<int>::iterator it = cell->getNetList().begin(); it != cell->getNetList().end(); ++it) {
    if (_netArray[*it]->getLock())
      continue;
    if (_netArray[*it]->getPartCount(0) == 0) {
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->incGain();
      }
    }
    if (_netArray[*it]->getPartCount(1) == 0) {
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->incGain();
      }
    }
    if (_netArray[*it]->getPartCount(0) == 1) {
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->decGain();
      }
    }
    if (_netArray[*it]->getPartCount(1) == 1) {
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->decGain();
      }
    }
  }
}

void Partitioner::move(Node *tar) {
  // update values
  _partSize[_cellArray[tar->getId()]->getPart()]--;
  for (vector<int>::iterator it = _cellArray[tar->getId()]->getNetList().begin(); it != _cellArray[tar->getId()]->getNetList().end(); ++it) {
    _netArray[*it]->decPartCount(_cellArray[tar->getId()]->getPart());
  }

  // cell part change
  _cellArray[tar->getId()]->move();
  _cellArray[tar->getId()]->lock();

  // update values
  _partSize[_cellArray[tar->getId()]->getPart()]++;
  for (vector<int>::iterator it = _cellArray[tar->getId()]->getNetList().begin(); it != _cellArray[tar->getId()]->getNetList().end(); ++it) {
    if (_netArray[*it]->getLock())
      continue;
    _netArray[*it]->lock(_cellArray[tar->getId()]->getPart());
    _netArray[*it]->incPartCount(_cellArray[tar->getId()]->getPart());
  }

  // remove cell from list
  if (_sList[_cellArray[tar->getId()]->getGain()] == tar) {
    _sList[_cellArray[tar->getId()]->getGain()] = tar->getNext();
    if (tar->getNext() == NULL) {
      _sList.erase(_cellArray[tar->getId()]->getGain());
      _sListtail.erase(_cellArray[tar->getId()]->getGain());
    }
  } else {
    tar->getPrev()->setNext(tar->getNext());
    tar->getNext()->setPrev(tar->getPrev());
  }

  // update accumulative gain and move record
  _accGain += _cellArray[tar->getId()]->getGain();
  if (_accGain > _maxAccGain) {
    _maxAccGain = _accGain;
    _bestMoveNum = _moveNum;
  }
  _moveNum++;
  _moveStack.push_back(tar->getId());

  // update gain
  updateGain(_cellArray[tar->getId()]);

  // add cell back to list
  if (_sList.find(_cellArray[tar->getId()]->getGain()) == _sList.end()) {
    _sList[_cellArray[tar->getId()]->getGain()] = tar;
    _sListtail[_cellArray[tar->getId()]->getGain()] = tar;
  } else {
    _sListtail[_cellArray[tar->getId()]->getGain()]->setNext(tar);
    tar->setPrev(_sListtail[_cellArray[tar->getId()]->getGain()]);
    _sListtail[_cellArray[tar->getId()]->getGain()] = tar;
  }
}

bool Partitioner::refine() {
  if (_maxAccGain <= 0)
    return false;
  for (int i = _moveNum - 1; i >= _bestMoveNum; i--) {
    int cellid = _moveStack[i];
    _cellArray[cellid]->move();
  }
  for (vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it) {
    (*it)->unlock(0);
    (*it)->unlock(1);
    if ((*it)->getPartCount(0) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->incGain();
      }
    }
    if ((*it)->getPartCount(1) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->incGain();
      }
    }
    if ((*it)->getPartCount(0) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->decGain();
      }
    }
    if ((*it)->getPartCount(1) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->decGain();
      }
    }
  }
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    (*it)->unlock();
  }
  _accGain = 0;
  _maxAccGain = 0;
  _moveNum = 0;
  _iterNum++;
  _moveStack.clear();
  return true;
}

void Partitioner::partition() {

  // init partition
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    if (_partSize[1] < _cellNum * (1 - _threshold)) {
      (*it)->setPart(1);
      for (vector<int>::iterator it2 = (*it)->getNetList().begin(); it2 != (*it)->getNetList().end(); ++it2) {
        _netArray[*it2]->incPartCount(1);
      }
      ++_partSize[1];
    } else {
      (*it)->setPart(0);
      for (vector<int>::iterator it2 = (*it)->getNetList().begin(); it2 != (*it)->getNetList().end(); ++it2) {
        _netArray[*it2]->incPartCount(0);
      }
      ++_partSize[0];
    }
  }
  // init gain calculation
  for (vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it) {
    if ((*it)->getPartCount(0) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->incGain();
      }
    }
    if ((*it)->getPartCount(1) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->incGain();
      }
    }
    if ((*it)->getPartCount(0) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->decGain();
      }
    }
    if ((*it)->getPartCount(1) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->decGain();
      }
    }
  }

  // init bucket list
  int idx = 0;
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    if (_sList.find((*it)->getGain()) == _bList->end()) {
      _sList[(*it)->getGain()] = (*it)->getNode();
      _sListtail[(*it)->getGain()] = (*it)->getNode();
    } else {
      addBefore((*it)->getPart(), (*it)->getNode(), (*it)->getGain());
    }
    idx++;
  }

  // start partitioning
  _maxAccGain = 0;
  _iterNum = 0;
  for (int i = 0; i < MAX_ITER; i++) {
    while (true) {
      Node *tar = NULL;
      map<int, Node *>::reverse_iterator l0 = _sList.rbegin();
      Node *cur = (*l0).second;
      while (tar == NULL && l0 != _sList.rend()) {
        if (_cellArray[cur->getId()]->getLock()) {
          l0++;
          if (l0 == _sList.rend())
            break;
          cur = (*l0).second;
        } else {
          if (_partSize[_cellArray[cur->getId()]->getPart()] - 1 > _threshold) {
            tar = cur;
          } else {
            cur = cur->getNext();
            if (cur == NULL) {
              l0++;
              if (l0 == _sList.rend())
                break;
              cur = (*l0).second;
            }
          }
        }
      }
      if (tar == NULL)
        break;
      else
        move(tar);
    }
    if (refine() == false)
      break;
  }
}

void Partitioner::printSummary() const {
  cout << endl;
  cout << "==================== Summary ====================" << endl;
  cout << " Cutsize: " << _cutSize << endl;
  cout << " Total cell number: " << _cellNum << endl;
  cout << " Total net number:  " << _netNum << endl;
  cout << " Cell Number of partition A: " << _partSize[0] << endl;
  cout << " Cell Number of partition B: " << _partSize[1] << endl;
  cout << "=================================================" << endl;
  cout << endl;
  return;
}

void Partitioner::reportNet() const {
  cout << "Number of nets: " << _netNum << endl;
  for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
    cout << setw(8) << _netArray[i]->getName() << ": ";
    vector<int> cellList = _netArray[i]->getCellList();
    for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
      cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
    }
    cout << endl;
  }
  return;
}

void Partitioner::reportCell() const {
  cout << "Number of cells: " << _cellNum << endl;
  for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
    cout << setw(8) << _cellArray[i]->getName() << ": ";
    vector<int> netList = _cellArray[i]->getNetList();
    for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
      cout << setw(8) << _netArray[netList[j]]->getName() << " ";
    }
    cout << endl;
  }
  return;
}

void Partitioner::writeResult(fstream &outFile) {
  stringstream buff;
  buff << _cutSize;
  outFile << "Cutsize = " << buff.str() << '\n';
  buff.str("");
  buff << _partSize[0];
  outFile << "G1 " << buff.str() << '\n';
  for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
    if (_cellArray[i]->getPart() == 0) {
      outFile << _cellArray[i]->getName() << " ";
    }
  }
  outFile << ";\n";
  buff.str("");
  buff << _partSize[1];
  outFile << "G2 " << buff.str() << '\n';
  for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
    if (_cellArray[i]->getPart() == 1) {
      outFile << _cellArray[i]->getName() << " ";
    }
  }
  outFile << ";\n";
  return;
}

void Partitioner::clear() {
  for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
    delete _cellArray[i];
  }
  for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
    delete _netArray[i];
  }
  return;
}
