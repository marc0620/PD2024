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

#define MAX_ITER 20
void Partitioner::parseInput(fstream &inFile) {
  string str;
  // Set balance factor
  inFile >> str;
  _bFactor = stod(str);



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
  _threshold = ((1 - _bFactor)*_cellNum) / 2;
  return;
}
void Partitioner::addBefore(int part, Node *node, int gain) {
  if (_sList.find(gain) == _sList.end()) {
    _sList[gain] = node;
    _sListtail[gain] = node;
    node->setNext(NULL);
    node->setPrev(NULL);
  } else {
    node->setNext(_sList[gain]);
    _sList[gain]->setPrev(node);
    _sList[gain] = node;
    node->setPrev(NULL);
  }
  return;
}

void Partitioner::addAfter(int part, Node *node, int gain) {
  if (_sList.find(gain) == _sList.end()) {
    _sList[gain] = node;
    _sListtail[gain] = node;
  } else {
    _sListtail[gain]->setNext(node);
    node->setPrev(_sListtail[gain]);
    _sListtail[gain] = node;
    node->setNext(NULL);
  }
  return;
}

void Partitioner::remove(Node *node, int gain) {
  if (node->getPrev() == NULL) {
    _sList[gain] = node->getNext();
    if (node->getNext() == NULL) {
      _sList.erase(gain);
      _sListtail.erase(gain);
    }else{
      node->getNext()->setPrev(NULL);
    }
  } else {
    node->getPrev()->setNext(node->getNext());
    if (node->getNext() != NULL) {
      node->getNext()->setPrev(node->getPrev());
    } else {
      _sListtail[gain] = node->getPrev();
    }
  }
  node->setPrev(NULL);
  node->setNext(NULL);
  return;
}


void Partitioner::updateGain(Cell *cell) {
  int to=cell->getPart();
  int from=1-to;
  for (vector<int>::iterator it = cell->getNetList().begin(); it != cell->getNetList().end(); ++it) {
    if (_netArray[*it]->getLock()){
      continue;
    }
    if(_netArray[*it]->getPartCount(from)==0){
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if(_affectedCell.find(*it2)==_affectedCell.end()){
          _affectedCell[*it2]= _cellArray[*it2]->getGain();
          //cout<<"append: "<<_cellArray[*it2]->getName()<<endl;
        }
        _cellArray[*it2]->decGain();
        if(_cellArray[*it2]==cell){
          _cellArray[*it2]->decGain();
        }
      }
    }
    if(_netArray[*it]->getPartCount(from)==1){
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if(_cellArray[*it2]->getPart()==from){
          if(_affectedCell.find(*it2)==_affectedCell.end()){
            _affectedCell[*it2] = _cellArray[*it2]->getGain();
          }
          _cellArray[*it2]->incGain();
          break;
        }
      }
    }
    if(_netArray[*it]->getPartCount(to)==1){
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if(_affectedCell.find(*it2)==_affectedCell.end()){
          _affectedCell[*it2]= _cellArray[*it2]->getGain();
        }
        if(_cellArray[*it2]->getPart()==from){
          _cellArray[*it2]->incGain();
        }else{
          _cellArray[*it2]->incGain();
          _cellArray[*it2]->incGain();
        }

      }
    }
    if(_netArray[*it]->getPartCount(to)==2){
      for (vector<int>::iterator it2 = _netArray[*it]->getCellList().begin(); it2 != _netArray[*it]->getCellList().end(); ++it2) {
        if(_cellArray[*it2]->getPart()==to &&_cellArray[*it2]!=cell){
          if(_affectedCell.find(*it2)==_affectedCell.end()){
            _affectedCell[*it2]= _cellArray[*it2]->getGain();
          }
          _cellArray[*it2]->decGain();
          break;
        }
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
    _netArray[*it]->incPartCount(_cellArray[tar->getId()]->getPart());
    if (_netArray[*it]->getLock())
      continue;
  }

  // remove cell from list
  remove(tar, _cellArray[tar->getId()]->getGain());

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
  for (vector<int>::iterator it = _cellArray[tar->getId()]->getNetList().begin(); it != _cellArray[tar->getId()]->getNetList().end(); ++it) {
    _netArray[*it]->lock(_cellArray[tar->getId()]->getPart());
  }
  //reportCell();
  // add cell back to list

  //modify affected cells
  for (map<int,int>::iterator it = _affectedCell.begin(); it != _affectedCell.end(); ++it) {
    if(_cellArray[it->first]->getLock()){
      continue;
    }
    if(it->second!=_cellArray[it->first]->getGain()){
      remove(_cellArray[it->first]->getNode(), it->second);
      addBefore(_cellArray[it->first]->getPart(), _cellArray[it->first]->getNode(), _cellArray[it->first]->getGain());
    }
  }
  _affectedCell.clear();
  return;
}


bool Partitioner::refine() {
  //reportNet();
  //cout<<"maxAccGain: "<<_maxAccGain<<endl;
  if (_maxAccGain <= 0)
    return false;
  _cutSize-=_maxAccGain;
  for (int i = _moveNum - 1; i > _bestMoveNum; i--) {
    //cout<<"revert: "<<_cellArray[_moveStack[i]]->getName()<<endl;
    int cellid = _moveStack[i];
    _cellArray[cellid]->move();
    _partSize[1-_cellArray[cellid]->getPart()]--;
    _partSize[_cellArray[cellid]->getPart()]++;
    for (vector<int>::iterator it = _cellArray[cellid]->getNetList().begin(); it != _cellArray[cellid]->getNetList().end(); ++it) {
      _netArray[*it]->incPartCount(_cellArray[cellid]->getPart());
      _netArray[*it]->decPartCount(1 - _cellArray[cellid]->getPart());
    }
  //reportNet();
  ////cout<<endl;
  }

  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    (*it)->unlock();
    (*it)->clearGain();
  }


  for (vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it) {
    (*it)->unlock();
    if ((*it)->getPartCount(0) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->decGain();
      }
    }
    if ((*it)->getPartCount(1) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->decGain();
      }
    }
    if ((*it)->getPartCount(0) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->incGain();
      }
    }
    if ((*it)->getPartCount(1) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->incGain();
      }
    }
  }
  //cout<<"partsize: "<<_partSize[0]<<" "<<_partSize[1]<<endl;
  //reportCell();
  while(!_sList.empty()){
    _sList.erase(_sList.begin());
    _sListtail.erase(_sListtail.begin());
  }
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    addBefore((*it)->getPart(), (*it)->getNode(), (*it)->getGain());
  }
  //printslist();
  _accGain = 0;
  _maxAccGain = 0;
  _moveNum = 0;
  _iterNum++;
  _moveStack.clear(); 


  return true;
}
void Partitioner::printslist() {
  for (map<int, Node *>::iterator it = _sList.begin(); it != _sList.end(); ++it) {
    cout << it->first << " ";
    Node *cur = it->second;
    while (cur != NULL) {
      cout << cur->getId() << " ";
      cur = cur->getNext();
    }
    cout<<"tail: "<<_sListtail[it->first]->getId()<<endl;
  }
  return;
}

void Partitioner::partition() {

  // init partition
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    if (_partSize[1] < _cellNum - _threshold) {
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
    //cout<< _partSize[0] << " " << _partSize[1] <<" "<<_cellNum*(1-_threshold) << endl;
  }
  // init gain calculation
  //reportNet();
  for (vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it) {

    if ((*it)->getPartCount(0) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->decGain();
      }
    }
    if ((*it)->getPartCount(1) == 0) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->decGain();
      }
    }
    if ((*it)->getPartCount(0) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 0)
          _cellArray[*it2]->incGain();
      }
    }
    if ((*it)->getPartCount(1) == 1) {
      for (vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2) {
        if (_cellArray[*it2]->getPart() == 1)
          _cellArray[*it2]->incGain();
      }
    }
    if((*it)->getPartCount(0)>0 && (*it)->getPartCount(1)>0){
      _cutSize++;
    }
  }

  // init bucket list
  int idx = 0;
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    addBefore((*it)->getPart(), (*it)->getNode(), (*it)->getGain());
    idx++;
  }
  // start partitioning
  _maxAccGain = 0;
  _iterNum = 0;
  for (int i = 0; i < MAX_ITER; i++) {
    cout<<"cutsize: "<<_cutSize<<endl;
    for (int j = 0; j < 1000000; j++) {
      //cout<<"accgain: "<<_accGain<<endl;
      //reportCell();
      Node *tar = NULL;
      map<int, Node *>::reverse_iterator l0 = _sList.rbegin();
      if(l0==_sList.rend())
        break;
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
      else{
        //cout<<"tar: "<<_cellArray[tar->getId()]->getName()<<endl;
        move(tar);
      }
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
    cout << setw(8) << _netArray[i]->getName() << ": "<< _netArray[i]->getPartCount(0) << " " << _netArray[i]->getPartCount(1) << endl;
    //vector<int> cellList = _netArray[i]->getCellList();
    //for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
    //  cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
    //}
    //cout << endl;
  }
  return;
}

void Partitioner::reportCell() const {
  cout << "Number of cells: " << _cellNum << endl;
  for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
    cout << setw(8) << _cellArray[i]->getName() << " "<< _cellArray[i]->getGain() << " " << _cellArray[i]->getPart() << " "<<endl;
    //vector<int> netList = _cellArray[i]->getNetList();
    //for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
    //  cout << setw(8) << _netArray[netList[j]]->getName() << " ";
    //}
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
