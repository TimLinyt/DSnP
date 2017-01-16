/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
unsigned CirGate::_globalRef = 1;

//size_t
void
CirPoGate::simulate() 
{
   size_t s0 = _in[0].gate()->getSim();
   if (_in[0].isInv()) s0 = ~s0;
   _simResult = s0;
}

void
CirAigGate::simulate()
{
   size_t s0 = _in[0].gate()->getSim();
   size_t s1 = _in[1].gate()->getSim();
   if (_in[0].isInv()) s0 = ~s0;
   if (_in[1].isInv()) s1 = ~s1;
   _simResult = s0 & s1;
}


bool 
CirGate::isfloat() 
{ 
   for (size_t z = 0, zn = _in.size(); z < zn; z++) {
      if (_in[z].gate()->getTypeStr() == "UNDEF") { 
         return true;
      }
   }
   return false;
}

void 
CirGate::dfsTraversal(IdList& dfsList) 
{
   for (size_t n = 0, nn = _in.size(); n < nn; n++) {
      if (!_in[n].gate()->isGlobalRef()) {
         _in[n].gate()->setToGlobalRef();
         _in[n].gate()->dfsTraversal(dfsList);
      }
   }
   if (getTypeStr() == "UNDEF") return;
   dfsList.push_back(_vId);
}

IdList
CirGate::getFaninlist() const {
   IdList temp;
   for (size_t n = 0, nn = _in.size(); n < nn; n++) {
      temp.push_back(_in[n].gate()->getId()*2 + _in[n].isInv());
   }
   return temp;
}

IdList
CirGate::getFanoutlist() const {
   IdList temp;
   for (size_t n = 0, nn = _out.size(); n < nn; n++) {
      temp.push_back(_out[n].gate()->getId()*2 + _out[n].isInv());
   }
   return temp;
}

void
CirGate::Fanin(int level, int nowlevel, bool isInv) 
{
   for (size_t nn = 0; nn < nowlevel; nn++) { cout << "  "; }
   if (isInv) cout << '!';
   cout << getTypeStr() << ' ' << _vId;
   if (level > nowlevel && _in.size() > 0) {
      if (isGlobalRef()) {
         cout << " (*)";
         cout << endl;
      }
      else {
         cout << endl;
         for (size_t n = 0; n < _in.size(); n++) {
            _in[n].gate()->Fanin(level, nowlevel+1, _in[n].isInv());
         }
      }
   }
   else cout << endl;
   setToGlobalRef();
}


void
CirGate::Fanout(int level, int nowlevel, bool isInv) 
{
   for (size_t nn = 0; nn < nowlevel; nn++) { cout << "  "; }
   if (isInv) cout << '!';
   cout << getTypeStr() << ' ' << _vId;
   if (level > nowlevel && _out.size() > 0) {
      if (isGlobalRef()) {
         cout << " (*)";
         cout << endl;
      }
      else {
         cout << endl;
         for (size_t n = 0; n < _out.size(); n++) {
            _out[n].gate()->Fanout(level, nowlevel+1, _out[n].isInv());
         }
      }
   }
   else cout << endl;
   setToGlobalRef();
}

void
CirGate::deleteOut(CirGate* g) 
{ 
   for (int i = _out.size() - 1; i >= 0; i--) {
      if (_out[i].gate() == g) {
         _out.erase(_out.begin() + i);
         break;
      }
   }
}

void
CirGate::replacein(CirGateV oldg, CirGateV newg)
{ 
   for (int i = 0; i < _in.size(); i++) {
      if (_in[i].gate() == oldg.gate() && _in[i].isInv() == oldg.isInv()) {
         _in[i] = newg;
         break;
      }
   }
}


void
CirGate::reportGate() const
{
   stringstream ss, sss, ssss;
   cout << "==================================================\n";
   ss << "= " << getTypeStr() << '(' << getId() << ')';
   if (_symbol != "") 
      ss << "\"" << getSymbolStr() << "\"";
   ss << ", line " << getLineNo();
   cout << setw(49) << left << ss.str() << "=\n";

   sss << "= FECs: ";
   size_t isinv = _simResult & 1;
   if (_fec) {
      for (size_t i = 0, in = _fec->size(); i < in; i++) {  
         if (((*_fec)[i]->getSim() & 1) != (isinv)) sss << "!";
         unsigned temp = (*_fec)[i]->getId();
         if(temp != _vId) sss << temp << " ";
      } 
   }
   cout << setw(49) << left << sss.str() << "=\n";

   ssss << "= Value: ";
   for (size_t i = 0; i < 32; i++) {
      if (i && i%4 == 0) ssss << "_";
      ssss << ((_simResult >> (i)) & 1);
   }
   cout << setw(49) << left << ssss.str() << "=\n";
   
   
   cout << "==================================================\n";
}

void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   setGlobalRef();
   Fanin(level, 0, 0);
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   setGlobalRef();
   Fanout(level, 0, 0);
}

//print gate functions
void 
CirPiGate::printGate() const 
{
   cout << "PI  " << _vId;
   if (_symbol != "") {
      cout << " (" << _symbol << ")";
   }
}

void 
CirAigGate::printGate() const 
{
   cout << "AIG " << _vId;
   for (size_t n = 0; n < 2; n++) {
      cout << ' ';
      if (_in[n].gate()->getTypeStr() == "UNDEF") cout << '*';
      if (_in[n].isInv()%2 == 1) cout << '!';
      cout << _in[n].gate()->getId();
   }
}


void 
CirPoGate::printGate() const 
{
   cout << "PO  " << _vId << ' ';
   if (_in[0].gate()->getTypeStr() == "UNDEF") cout << '*';
   if (_in[0].isInv()%2 == 1) cout << '!';
   cout << _in[0].gate()->getId();
   if (_symbol != "") {
      cout << " (" << _symbol << ")";
   }
}


void 
CirUndefGate::printGate() const { return; }


void 
CirConstGate::printGate() const 
{
   cout << "CONST0";
}

//construct functions
CirPiGate::CirPiGate(unsigned vid, unsigned ln)
{
   _vId = vid;
   _lineno = ln;   
}

CirAigGate::CirAigGate(unsigned vid, unsigned ln)
{
   _vId = vid;
   _lineno = ln;
}

CirPoGate::CirPoGate(unsigned vid, unsigned ln)
{
   _vId = vid;
   _lineno = ln;
}

CirUndefGate::CirUndefGate(unsigned id)
{
   _vId = id;
}
 
CirConstGate::CirConstGate()
{
   _vId = 0;
}
