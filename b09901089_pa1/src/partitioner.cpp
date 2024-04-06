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
#include <time.h>
using namespace std;


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
int Partitioner::calculateGain(bool unlocknet){
  int cutsize=0;
  for (vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it) {
    if(unlocknet)
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
    if((*it)->getPartCount(0)>0 && (*it)->getPartCount(1)>0){
      cutsize++;
    }
  }
  return cutsize;
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
        }
        _cellArray[*it2]->decGain();
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

void Partitioner::move(Node *tar, int mode) {
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
  if(mode==1){
    if (_accGain > _maxAccGain) {
      _maxAccGain = _accGain;
      _bestMoveNum = _moveNum;  
    }
  }else if(mode==0){
    if (_accGain > _maxAccGain || (_partSize[0] == _cellNum-_threshold)) {
      _maxAccGain = _accGain;
      _bestMoveNum = _moveNum;
    }
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


bool Partitioner::refine(int mode) {
  //reportNet();
  //cout<<"maxAccGain: "<<_maxAccGain<<endl;
  if(mode==1){
    if((_maxAccGain<= _initcutsize*_cutRatio) && _perturbRatio>0){
      if(_maxAccGain>0){
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
        }
      }
      restore();
      //if(_cutSize!=calculateCutSize()){
      //  cout<<"cutsize error"<<endl;
      //}else{
      //  cout<<"correct cutsize: "<<_cutSize<<endl;
      //}
      perturb();
      
      //cout<<"perturb cutsize: "<<_cutSize<<endl;
      //cout<<"perturb ratio: "<<_perturbRatio<<endl;
      setup(true);
      return true;
    }
    if(_perturbRatio<=0){
      _iterNum++;
      if(_maxAccGain<=0){
        return false;
      }
    }
  }
  //cout<<"_accmax "<<_maxAccGain<< " cutsize: "<<_cutSize<<endl;
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
  }

  setup(true);
  //if(_cutSize!=calculateCutSize()){
  //  cout<<"cutsize error"<<endl;
  //  cout<<"real cutsize: "<<calculateCutSize() <<" cutsize "<< _cutSize<<endl;
  //}
  //cout<<"cutsize: "<<_cutSize<<"\n";


  return true;
}

void Partitioner::perturb(){

  if(_perturbRatio-_perturbStep<=0 && _perturbNum%_perturbPeriod==_perturbPeriod-1){
    _perturbRatio-=_perturbStep;

    return;
  }
  //cout<<"perturb "<<_perturbRatio<<endl;
  int count=0;
  for(vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it){
    (*it)->unlock();
  }
  while(count<_cellNum*_perturbRatio){
    int idx = rand()%_cellNum;
    if(_partSize[_cellArray[idx]->getPart()]-1 <= _threshold || _cellArray[idx]->getLock()){
      continue;
    }
    _partSize[_cellArray[idx]->getPart()]--;
    _cellArray[idx]->move();
    _partSize[_cellArray[idx]->getPart()]++;
    _cellArray[idx]->lock();
    count++;
  }
  //for(vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it){
  //  if(rand()%100<_perturbRatio*100 && _partSize[(*it)->getPart()]-1 > _threshold){
  //    _partSize[(*it)->getPart()]--;
  //    (*it)->move();
  //    _partSize[(*it)->getPart()]++;
  //  }
  //}
  _cutSize=0;
  for(vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
    (*it)->setPartCount(0,0);
    (*it)->setPartCount(1,0);
    bool p[2]={false,false};
    for(vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2){
      (*it)->incPartCount(_cellArray[*it2]->getPart());
      p[_cellArray[*it2]->getPart()]=true;
    }
    if(p[0] && p[1]){
      _cutSize++;
    }
  }

  if(_perturbNum%_perturbPeriod==_perturbPeriod-1){
    _perturbRatio-=_perturbStep;
  }
  _perturbNum++;
}

void Partitioner::restore(){
  if(_cutSize<_bestCut || _bestCut<0){
    _bestCut=_cutSize;
    for(vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it){
      (*it)->setBestPart((*it)->getPart());
    }
    //cout<<"bestcut: "<<_bestCut<<endl;
  }else{
    for(vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it){
      (*it)->setPart((*it)->getBestPart());
    }
    //cout << "restore current: " << _cutSize <<" best "<<_bestCut << endl;
    _cutSize=_bestCut;
  }

  for(vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
    (*it)->setPartCount(0,0);
    (*it)->setPartCount(1,0);
    for(vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2){
      (*it)->incPartCount(_cellArray[*it2]->getPart());
    }
  }
  _partSize[0]=0;
  _partSize[1]=0;
  for(vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it)
    _partSize[(*it)->getPart()]++;
  
}

void Partitioner::setup(bool increase_iter){
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    (*it)->unlock();
    (*it)->clearGain();
  }
  calculateGain(true);
  _sList.clear();
  _sListtail.clear();
  for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
    addBefore((*it)->getPart(), (*it)->getNode(), (*it)->getGain());
  }
  _accGain = 0;
  _maxAccGain = 0;
  _moveNum = 0;
  _moveStack.clear(); 
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

void Partitioner::initialPartition(int mode){
  if(mode==0){
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
    }
    _cutSize=calculateGain(false);
    for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
      addBefore((*it)->getPart(), (*it)->getNode(), (*it)->getGain());
    }
  }
  else if(mode==1){
    for(vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it){
      (*it)->setPart(0);
    }
    for(vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
      (*it)->setPartCount(0,(*it)->getCellList().size());
    }
    _partSize[0]=_cellNum;
    _partSize[1]=0;
    _cutSize=0;
    calculateGain(false);
    int idx = 0;
    for (vector<Cell *>::iterator it = _cellArray.begin(); it != _cellArray.end(); ++it) {
      addBefore((*it)->getPart(), (*it)->getNode(), (*it)->getGain());
      idx++;
    }
    // start partitioning
    _maxAccGain = 0;
    for(int i=0;i<_cellNum;i++){
      Node *tar = NULL;
      map<int, Node *>::reverse_iterator l0 = _sList.rbegin();
      if(l0==_sList.rend()){
        //cout<<"slist empty "<<i<<endl;
        break;
      }
      Node *cur = (*l0).second;
      if(_partSize[0] - 1 <= _threshold){
        break;
      }else{
        tar = cur;
      }
      if (tar == NULL){
        //cout<<"no tar "<<i<<endl;
        break;
      }
      else{
        move(tar,0);
      }
      //cout<<"_accgain "<< _accGain<<endl;
      //cout<<"partsize0 "<<_partSize[0]<<"threshold "<< _cellNum-_threshold<<endl;
    }
    refine(0);
    _initcutsize=_cutSize;
    cout<<"initial cutsize: "<<_cutSize<<endl;

    //bao tiao parameter
    if(_initcutsize<2000){
      _perturbStep=0.05;
      _perturbRatio=0.4;
      _perturbPeriod=1;
      _cutRatio=0.001;
    }
    else if(_initcutsize<3000){
      _perturbStep=0.1;
      _perturbRatio=0.4;
      _perturbPeriod=2;
      _cutRatio=0.01;
    }
    else if(_initcutsize<4000){
      _perturbStep=0.4;
      _perturbRatio=0.15;
      _perturbPeriod=5;
      _cutRatio=0;
    }else if(_initcutsize<30000){
      _perturbStep=0.015;
      _perturbRatio=0.2;
      _perturbPeriod=1;
      _maxIter=5;
      _cutRatio=0.003;
    }else if(_initcutsize<50000){
      _perturbStep=0.03;
      _perturbRatio=0.3;
      _perturbPeriod=1;
      _maxIter=5;
      _cutRatio=0.005;
    }else{
      _perturbStep=0.04;
      _perturbRatio=0.3;
      _perturbPeriod=3;
      _maxIter=10;
      _cutRatio=0.006;
    }
    //cout<<_cutSize<< " " <<_partSize[0] << " "<<  _partSize[1]<<endl;
  }
}

int Partitioner::calculateCutSize(){
  int size=0;
  for(vector<Net *>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
    bool p[2]={false,false};
    for(vector<int>::iterator it2 = (*it)->getCellList().begin(); it2 != (*it)->getCellList().end(); ++it2){
      p[_cellArray[*it2]->getPart()]=true;
    }
    if(p[0] && p[1]){
      size++;
    }
  }
  return size;
}

void Partitioner::partition() {
  // init partition
  initialPartition(1);
  // start partitioning
  _maxAccGain = 0;
  _iterNum = 0;
  while(_iterNum<_maxIter) {
    //cout<<"cutsize: "<<_cutSize<<endl;
    for (int j = 0; j < _cellNum; j++) {
      //cout<<"accgain: "<<_accGain<<endl;
      //reportCell();
      Node *tar = NULL;
      map<int, Node *>::reverse_iterator l0 = _sList.rbegin();
      if(l0==_sList.rend())
        break;
      Node *cur = (*l0).second;
      while (tar == NULL && l0 != _sList.rend()) {
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

      if (tar == NULL)
        break;
      else{
        move(tar, 1);
      }
    }
    if (refine(1) == false)
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
