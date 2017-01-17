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
   HashSet<StrashNode> shash( getHashSize(_a));

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
   vector<size_t> pattern;
   vector< vector<size_t> > SATpattern;
   int ptnCounter = 0;
   bool changeorder = false;
   SatSolver solver;
   solver.initialize();
   //init
   InitSAT(solver);
   pattern.resize(_i);
   //sort
   sortFecGrps();
   
   UpdateFec();
   printFECPairs();



   /*while(_fecGrps.size() > 0) {
      cerr << "FEC size = "<< _fecGrps.size() << endl;
      for (size_t n = 0, nn = _fecGrps.size(); n < nn; n++) {
         
         for (size_t i = 0, in = _fecGrps[n].size(); i < in; i++) { 
            if(!_fecGrps[n][i]) continue;
            for (size_t j = i+1, jn = _fecGrps[n].size(); j < jn; j++) {
               if(!_fecGrps[n][j]) continue;
               if (proofFECpair(solver, _fecGrps[n][i], _fecGrps[n][j])) {
                  //merge i j
                  if (_fecGrps[n][i]->getTypeStr() == "AIG") {
                     IdList ilist = _fecGrps[n][i]->getFaninlist();
                     if (_fecGrps[n][j]->getId() == ilist[0]/2 ||
                        _fecGrps[n][j]->getId() == ilist[1]/2 ) {
                        changeorder  = true;
                     }
                  }

                  if (!changeorder) {
                     fraigMerge(_fecGrps[n][i], _fecGrps[n][j], 
                        (_fecGrps[n][i]->getSim() == _fecGrps[n][j]->getSim()));
                     _fecGrps[n][j] = 0;
                  }
                  else{
                        fraigMerge(_fecGrps[n][j], _fecGrps[n][i], 
                        (_fecGrps[n][j]->getSim() == _fecGrps[n][i]->getSim()));
                     _fecGrps[n][i] = 0;
                  }

               }
               else {
                  getSatAssignment(solver, pattern, ptnCounter);
                  if (ptnCounter%64 == 0) SATpattern.push_back(pattern);

               }
            }
         }
         for (size_t r = 0, rn = SATpattern.size(); r < rn; r++) {
            for (size_t i = 0; i < _i; i ++) {
               _gates[i]->setpisim(SATpattern[r][i]);
            }
            deleteinvaildFEC();
            updateDfsList();
            simulate();
            identifyFEC();
         }
         SATpattern.clear();
      }
      
      for (size_t i = 0; i < _i; i ++) {
         _gates[i]->setpisim(pattern[i]);
      }
      deleteinvaildFEC();
      updateDfsList();
      simulate();
      identifyFEC();
      UpdateFec();
      
      
   }
*/
   
/*  for (size_t i = 0; i < _fecGrps[10].size(); i++){
   cerr << i << " = " <<_fecGrps[10][i]->getId() << endl;
   }
   //
   for (size_t i = 0; i < _fecGrps[10].size(); i++){
   cerr << i << " = " <<_fecGrps[10][i]->getId() << endl;
   }
*/
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/   
void
CirMgr::deleteinvaildFEC() {
   for (size_t i = 0, in = _fecGrps.size(); i < in; i++) {
      for (int j = _fecGrps[i].size() - 1; j >= 0; j --) {
         if (_fecGrps[i][j] == 0) {
            _fecGrps[i].erase(_fecGrps[i].begin()+ j);
         }
      }
   }
}


void
CirMgr::fraigMerge(CirGate* master, CirGate* slave, bool inv)
{
   int temp = slave->getId();
   cout << "Fraig: " << master->getId() << " merging " ;
   cout << ((inv)? "" : "!") << slave->getId() << "..." << endl;
   
   IdList ilist = slave->getFaninlist();
   _vidgates[ilist[0]/2]->deleteOut(slave);
   _vidgates[ilist[1]/2]->deleteOut(slave);
   
   IdList slist = slave->getFanoutlist();
   for (int z = 0, zn = slist.size(); z < zn; z++) {
      _vidgates[slist[z]/2]->replacein(
            CirGateV(slave, slist[z]%2),
            CirGateV(master, (bool)(slist[z]%2)^inv));
      master->setOut(CirGateV(_vidgates[slist[z]/2], slist[z]%2));
   }
   delete _vidgates[temp];
   _vidgates[temp] = 0;
}

bool
CirMgr::proofFECpair(SatSolver& solver, CirGate* g1, CirGate* g2)
{
   Var F = solver.newVar();
   solver.addXorCNF(F, g1->getVar(), false, g2->getVar(), (g1->getSim() != g2->getSim()) );
   solver.assumeRelease();  // Clear assumptions
   solver.assumeProperty( _vidgates[0]->getVar(), false );
   solver.assumeProperty(F, true);  // k = 1
   return !solver.assumpSolve();
}

void 
CirMgr::getSatAssignment(SatSolver& solver, vector<size_t>& ptn, int& count)
{
   for (size_t i = 0, in = _i ; i < in ;i++) {
      ptn[i] = (ptn[i] << 1);
      ptn[i] += solver.getValue(_gates[i]->getVar());
   }
   count++;
}


bool cmpfecdis(pair<unsigned,GateList> i, pair<unsigned,GateList> j) 
{
   return (i.first < j.first); 
}  

void
CirMgr::sortFecGrps()
{
   for (size_t i = 0, in = _dfsList.size(); i < in; i++) {
      if (_vidgates[_dfsList[i]]->getTypeStr() == "AIG") {
         unsigned u0, u1;
         IdList ilist = _vidgates[_dfsList[i]]->getFaninlist();
         u0 = _vidgates[ilist[0]/2]->get_d();
         u1 = _vidgates[ilist[1]/2]->get_d();
         _vidgates[_dfsList[i]]->set_d( u0 + u1 +1 );/////
      }
      else _vidgates[_dfsList[i]]->set_d(0);
   }

   unsigned d = 0;
   vector< pair<unsigned, GateList> > templist;
   templist.resize(_fecGrps.size());
   for (size_t i = 0, in = _fecGrps.size(); i < in; i++) {
      d = 0;
      for (size_t j = 0, jn = _fecGrps[i].size(); j < jn; j++) {
         d += _fecGrps[i][j]->get_d();
      }
      templist[i] = pair<unsigned, GateList>(d, _fecGrps[i]);
   }
   sort(templist.begin(), templist.end(), cmpfecdis);
   for (size_t i = 0, in = _fecGrps.size(); i < in ; i++) {
      _fecGrps[i] = templist[i].second;
   }
   UpdateFec();
}

void 
CirMgr::InitSAT(SatSolver& solver) 
{
   Var v = solver.newVar();
   _vidgates[0]->setVar(v);
   for (size_t i = 0, in = _dfsList.size(); i < in; i++) {
      if (_vidgates[_dfsList[i]]->getTypeStr() != "PO") {
         v = solver.newVar();
         _vidgates[_dfsList[i]]->setVar(v);
         if (_vidgates[_dfsList[i]]->getTypeStr() == "AIG") {
            IdList ilist = _vidgates[_dfsList[i]]->getFaninlist();
            solver.addAigCNF(_vidgates[_dfsList[i]]->getVar(), 
                             _vidgates[ilist[0]/2]->getVar(), ilist[0]%2, 
                             _vidgates[ilist[1]/2]->getVar(), ilist[1]%2);
         }
      }
   }
}

void
CirMgr::MergeGate(CirGate* master, CirGate* slave) 
{
   int temp = slave->getId();
   cout << "Strashing: " << master->getId() << " merging " ;
   cout << slave->getId() << "..." << endl;
   
   IdList ilist = slave->getFaninlist();
   _vidgates[ilist[0]/2]->deleteOut(slave);
   _vidgates[ilist[1]/2]->deleteOut(slave);
   
   IdList slist = slave->getFanoutlist();
   for (int z = 0, zn = slist.size(); z < zn; z++) {
      _vidgates[slist[z]/2]->replacein(
            CirGateV(slave, slist[z]%2),
            CirGateV(master, slist[z]%2));
      master->setOut(CirGateV(_vidgates[slist[z]/2], slist[z]%2));
   }
   delete _vidgates[temp];
   _vidgates[temp] = 0;
}
   
