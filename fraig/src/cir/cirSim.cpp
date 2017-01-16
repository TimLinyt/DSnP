/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <bitset>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "../util/myHashSet.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
class SimNode
{
public:
   SimNode() {}
   SimNode(CirGate* g) { 
      _gate = g; 
      _value = _gate->getSim();
      if (_value & 1) _value = ~_value;
      //_isInv = (_value & 1);
   }
   ~SimNode() {};

   size_t operator() () const {
      //if (_isInv) return ~_value;
      //else return _value;
      return _value;
   }

   bool operator == (const SimNode& s) const { 
      //return ((_isInv) ? (~_value == s()) : (_value == s())); 
      return _value == s._value;
   }

   SimNode& operator = (const SimNode& s) {
      _gate = s._gate;
      _value = s._value;
      //_isInv = s._isInv;
      return *this;
   }

   
   CirGate* _gate;
private:
   size_t _value;
   //bool _isInv;
};

bool comparefec (CirGate* i, CirGate* j) { return (i->getId() < j->getId()); }
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{

}

void
CirMgr::fileSim(ifstream& patternFile)
{
   string   str;
   vector<string> ptns;
   ptns.resize(64);
   int counter = 0;
   fecInit();
   while (patternFile >> str) {
      if (!checkerr(str)) return;
      //ptns.push_back(str);
      ptns[counter%64] = str;
      counter ++;
      
      if (counter%64 == 0) {
         //ptn to sim
         ptnToSim(ptns);  
         simulate();
         //FEC
         identifyFEC(); 
         //write log
         if (_simLog) writeLog(ptns);
         //ptns.clear(); 
      }  
   }
   
   if (counter%64 != 0) {
      //ptn to sim
      ptnToSim(ptns, counter%64);  
      simulate();
      //FEC
      identifyFEC();
      //write log
      if (_simLog) writeLog(ptns);
      //ptns.clear();
   }
   
   for (size_t i = 0, in = _fecGrps.size(); i < in; i++ ) {
      sort( _fecGrps[i].begin(), _fecGrps[i].end(), comparefec);
   }

   CirGate::setGlobalRef();
   for (unsigned i = 0, in = _fecGrps.size(); i < in; i++ ) {
      for (unsigned j = 0, jn = _fecGrps[i].size(); j < jn; j++) {
         _fecGrps[i][j]->setfec(&_fecGrps[i]);
         _fecGrps[i][j]->setToGlobalRef();
      }
   }
   for (size_t i = 0; i <= _m; i++) {
      if(_vidgates[i]) {
         if(!_vidgates[i]->isGlobalRef()) {
            _vidgates[i]->setfec(0);
         }
      }
   }
   

   cout << counter << " patterns simulated." << endl;
/*for (size_t i = 0; i < _fecGrps.size(); i++) {
   for (size_t j = 0; j < _fecGrps[i].size(); j++) {
      cerr << _fecGrps[i][j]->getId() << " ";
   }
   cerr << endl;
}*/
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void
CirMgr::fecInit()
{
   if (_fecGrps.size() == 0) {
      GateList initfecGrp;
      initfecGrp.push_back(_vidgates[0]);
      for (size_t i = 0, in = _dfsList.size(); i < in; i++) {
         if (_vidgates[_dfsList[i]]->getTypeStr() == "AIG") {
            initfecGrp.push_back(_vidgates[_dfsList[i]]);
         }
      }
      _fecGrps.push_back(initfecGrp);
   }
}

void
CirMgr::identifyFEC()
{
   vector<GateList> tempGrps;
   unsigned counter = 0;

   CirGate::setGlobalRef();
   
   for (size_t i = 0, in = _fecGrps.size(); i < in; i++) {
      //trival condition -> no need to use hash
      if (_fecGrps[i].size() == 2) {
         SimNode sNode0(_fecGrps[i][0]), sNode1(_fecGrps[i][1]);
         if (sNode0 == sNode1) {
            GateList grp;
            grp.push_back(_fecGrps[i][0]);
            grp.push_back(_fecGrps[i][1]);
            tempGrps.push_back(grp);
            counter++;
         }
         continue;
      }
      HashSet<SimNode> newFecGrps( getHashSize(_fecGrps[i].size()) );
      //each grp
      for (size_t j = 0, jn = _fecGrps[i].size(); j < jn; j++) {
         SimNode sNode = SimNode(_fecGrps[i][j]);
         if (newFecGrps.query(sNode)) { // already exist 
            //create grp
            if (!sNode._gate->isGlobalRef()) {
               sNode._gate->setToGlobalRef();
               GateList grp;
               grp.push_back(sNode._gate);
               grp.push_back(_fecGrps[i][j]);
               sNode._gate->setgrpNo(counter);
               tempGrps.push_back(grp);
               counter ++;
            }
            //add to grp
            else {
               tempGrps[sNode._gate->getgrpNo()].push_back(_fecGrps[i][j]);
            }
         }
         else { // add to hash
            newFecGrps.insert(sNode);
         }
      }
   }
   _fecGrps.clear();
   _fecGrps.swap(tempGrps);

   cout << "Total #FEC Group = " << _fecGrps.size() << char(13) << flush;;
}


   
void 
CirMgr::ptnToSim (vector<string>& ptns, size_t count) 
{
   size_t n = count;
   for (size_t i = 0; i < _i; i ++) {
      size_t temp = 0;
      for (size_t j = 0; j < n; j++) {
         temp = (temp << 1) | (size_t)(ptns[j][i] - '0');
      }
      _vidgates[i+1]->setpisim(temp);
   }

}

void
CirMgr::writeLog(vector<string>& ptns)
{
   size_t n = ptns.size();
   for (size_t i = 0; i < n; i++) {
      *_simLog << ptns[i] << " ";
      for (size_t j = 1; j <= _o; j++) { 
         size_t temp = _vidgates[_m+j]->getSim();
         *_simLog << ((temp >> (n - i - 1)) & 1 );
      }
      *_simLog << endl;
   }
}

bool
CirMgr::checkerr(string & str) 
{
   if (str.length() != _i) {
      cerr << "Error: Pattern(" << str << ") length(" << str.length();
      cerr << ") does not match the number of inputs(" << _i;
      cerr << ") in a circuit!!" << endl;
      return false;
   }
   for (size_t i = 0; i < _i; i++) {
      if (str[i] != '0' && str[i] != '1') {
         cerr << "Error: Pattern(" << str << ") contains a non-0/1";
         cerr << " character(‘" << str[i] << cerr << "’)." << endl;
         return false;
      }
   }
   return true;
}

void 
CirMgr::simulate() 
{
   //CirGate::setGlobalRef();
   //for (size_t i = 1; i <= _i; i++) {
   //   _vidgates[i]->setToGlobalRef();
   //}
   for (size_t i = 0, n = _dfsList.size(); i < n; i++) {
      _vidgates[_dfsList[i]]->simulate();
   }
}
