/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   updateDfsList();
   for (size_t i = 1; i <= _m; i++) {
      if (_vidgates[i]) {
         if (_vidgates[i]->getTypeStr() == "PI") {}
         else if (_vidgates[i]->isGlobalRef()) {}
         else {
            cout << "Sweeping: " << _vidgates[i]->getTypeStr() << "(" 
               << i << ") removed..." << endl;
            IdList ls = _vidgates[i]->getFaninlist();
            for (size_t j = 0; j < ls.size(); j++) {
               if (_vidgates[ls[j]/2])
                  _vidgates[ls[j]/2]->deleteOut(_vidgates[i]);
            }
            delete _vidgates[i];
            _vidgates[i] = 0;
         }
      }
   }
   updateFloating();
   updateUnused();
}


// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   updateDfsList();
   for (size_t i = 0; i < _dfsList.size(); i++) {
      if (_vidgates[_dfsList[i]]->getTypeStr() == "AIG") {
         //cerr << "now id = " << _dfsList[i] << endl; 
         IdList ls = _vidgates[_dfsList[i]]->getFaninlist();
         //case 1 : same
         //cerr << "ls[0] = " << ls[0] << ", ls[1] = "<< ls[1] << endl; 
         if (ls[0] == ls[1]) {
            IdList olist = _vidgates[_dfsList[i]]->getFanoutlist();
            cout << "Simplifying: " << ls[0]/2 << " merging "; 
            cout << (ls[0]%2 ? "!" : "") << _dfsList[i] << "..." << endl;
            for (int z = 0; z < olist.size(); z++) {
               _vidgates[olist[z]/2]->replacein(
                     CirGateV(_vidgates[_dfsList[i]], olist[z]%2),
                     CirGateV(_vidgates[ls[0]/2], (ls[0]+olist[z])%2));
               _vidgates[ls[0]/2]->setOut(
                     CirGateV(_vidgates[olist[z]/2], (ls[0]+olist[z])%2));
            }
         }
         //case 2 : const 0
         else if (ls[0] == 0 || ls[1] == 0) {
            IdList olist = _vidgates[_dfsList[i]]->getFanoutlist();
            cout << "Simplifying: 0 merging "; 
            cout << _dfsList[i] << "..." << endl;           
            for (int z = 0; z < olist.size(); z++) {
               _vidgates[olist[z]/2]->replacein(
                     CirGateV(_vidgates[_dfsList[i]], olist[z]%2),
                     CirGateV(_vidgates[0], olist[z]%2));
               _vidgates[0]->setOut(CirGateV(_vidgates[olist[z]/2], olist[z]%2));
            }
         }
         //case 3 : const 1
         else if (ls[0] == 1 || ls[1] == 1) {
            IdList olist = _vidgates[_dfsList[i]]->getFanoutlist();
            int y = (ls[0] == 1)? 1 : 0;
            cout << "Simplifying: " << ls[y]/2 << " merging "; 
            cout << (ls[y]%2 ? "!" : "") << _dfsList[i] << "..." << endl;
            for (int z = 0; z < olist.size(); z++) {
               _vidgates[olist[z]/2]->replacein(
                     CirGateV(_vidgates[_dfsList[i]], olist[z]%2),
                     CirGateV(_vidgates[ls[y]/2], (ls[y]+olist[z])%2));
               _vidgates[ls[y]/2]->setOut(
                     CirGateV(_vidgates[olist[z]/2], (ls[y]+olist[z])%2));
            } 
         }
         //case 4 : inverse
         else if (ls[0]/2 == ls[1]/2) {
            IdList olist = _vidgates[_dfsList[i]]->getFanoutlist();
            cout << "Simplifying: 0 merging "; 
            cout << _dfsList[i] << "..." << endl;           
            for (int z = 0; z < olist.size(); z++) {
               _vidgates[olist[z]/2]->replacein(
                     CirGateV(_vidgates[_dfsList[i]], olist[z]%2),
                     CirGateV(_vidgates[0], olist[z]%2));
               _vidgates[0]->setOut(CirGateV(_vidgates[olist[z]/2], olist[z]%2));
            }
         }
         else { continue; }

         _vidgates[ls[0]/2]->deleteOut(_vidgates[_dfsList[i]]);
         _vidgates[ls[1]/2]->deleteOut(_vidgates[_dfsList[i]]);
         delete _vidgates[_dfsList[i]];
         _vidgates[_dfsList[i]] = 0;
      }
   }
   
   updateDfsList();
   updateFloating();
   updateUnused();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/


