/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

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
   int counter = 0;

   while (patternFile >> str) {
      if (!checkerr(str)) return;
      ptns.push_back(str);
      counter ++;
      
      if (counter%32 == 0) {
         //ptn to sim
         ptnToSim(ptns);  
         simulate();
         //FEC
      
         //write log
         if (_simLog) writeLog(ptns);
         ptns.clear();
      } 
   }
   
   if (counter%32 != 0) {
      //ptn to sim
      ptnToSim(ptns);  
      simulate();
      //FEC

      //write log
      if (_simLog) writeLog(ptns);
      ptns.clear();
   }

   cout << counter << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void 
CirMgr::ptnToSim (vector<string>& ptns) 
{
   size_t n = ptns.size();
   for (size_t i = 0; i < _i; i ++) {
      unsigned temp = 0;
      for (size_t j = 0; j < n; j++) {
         temp = (temp << 1) | (unsigned)(ptns[j][i] - '0');
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
         unsigned temp = _vidgates[_m+j]->getSim();
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
   CirGate::setGlobalRef();
   for (size_t i = 1; i <= _i; i++) {
      _vidgates[i]->setToGlobalRef();
   }
   for (size_t i = 0, n = _dfsList.size(); i < n; i++) {
      _vidgates[_dfsList[i]]->simulate();
   }
}
