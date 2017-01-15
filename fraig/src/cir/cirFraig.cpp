/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "../util/myHashSet.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
class StrashNode
{
public:
   StrashNode() {}
   StrashNode(CirGate* g) { _aiggate = g; }
   ~StrashNode() {};

   size_t operator() () const {
      IdList list = _aiggate->getFaninlist();
      size_t temp1 = (list[0] < list[1]) ? list[0] : list[1];
      size_t temp2 = (list[0] < list[1]) ? list[1] : list[0];
      return (temp1*10007 + temp2);
   }
   bool operator == (const StrashNode& s) const {
      IdList l1 = s._aiggate->getFaninlist();
      IdList l2 = _aiggate->getFaninlist();
      if (l1[0] == l2[0] && l1[1] == l2[1]) return true;
      if (l1[0] == l2[1] && l1[1] == l2[0]) return true;
      return false;
   }
   StrashNode& operator = (const StrashNode& s) {
      _aiggate = s._aiggate;
      return *(this);
   }
   
   CirGate* _aiggate;
private:

};


/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   HashSet<StrashNode> shash( _a * 10);

   for (size_t i = 1; i <= _m; i++) {
      if (_vidgates[i]) {
         if (_vidgates[i]->getTypeStr() == "AIG") {
            StrashNode sNode = StrashNode(_vidgates[i]);
            if (shash.query(sNode)) {
               MergeGate(sNode._aiggate, _vidgates[i]);
            }
            else {
               shash.insert(sNode);
            }
         }
      }
   }
   updateDfsList();
   updateFloating();
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirMgr::MergeGate(CirGate* master, CirGate* slave) 
{
   int temp = slave->getId();
   cout << "Strashing: " << master->getId() << " merging " ;
   cout << slave->getId() << "..." << endl;
   IdList slist = slave->getFanoutlist();
   for (int z = 0; z < slist.size(); z++) {
      _vidgates[slist[z]/2]->replacein(
            CirGateV(slave, slist[z]%2),
            CirGateV(master, slist[z]%2));
      master->setOut(CirGateV(_vidgates[slist[z]/2], slist[z]%2));
   }
   delete _vidgates[temp];
   _vidgates[temp] = 0;
}
   
