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

bool 
CirGate::isfloat() { 
   for (size_t z = 0; z < _in.size(); z++) {
      if (_in[z].gate()->getTypeStr() == "UNDEF") { 
         return true;
      }
   }
   return false;
}

void 
CirGate::dfsTraversal(IdList& dfsList) 
{
   for (size_t n = 0; n < _in.size(); n++) {
      if (!_in[n].gate()->isGlobalRef()) {
         _in[n].gate()->setToGlobalRef();
         _in[n].gate()->dfsTraversal(dfsList);
      }
   }
   if (_type == "UNDEF") return;
   dfsList.push_back(_vId);
}

IdList
CirGate::getFaninlist() const {
   IdList temp;
   for (size_t n = 0; n < _in.size(); n++) {
      temp.push_back(_in[n].gate()->getId()*2 + _in[n].isInv());
   }
   return temp;
}

IdList
CirGate::getFanoutlist() const {
   IdList temp;
   for (size_t n = 0; n < _out.size(); n++) {
      temp.push_back(_out[n].gate()->getId()*2 + _out[n].isInv());
   }
   return temp;
}

void
CirGate::Fanin(int level, int nowlevel, bool isInv) 
{
   for (size_t nn = 0; nn < nowlevel; nn++) { cout << "  "; }
   if (isInv) cout << '!';
   cout << _type << ' ' << _vId;
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
   cout << _type << ' ' << _vId;
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
CirGate::reportGate() const
{
   stringstream ss;
   ss << "= " << getTypeStr() << '(' << getId() << ')';
   if (_symbol != "") 
      ss << "\"" << getSymbolStr() << "\"";
   ss << ", line" << getLineNo();
   cout << "==================================================\n";
   cout << setw(49) << left << ss.str() << "=\n";
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
   _type = "PI";
   _vId = vid;
   _lineno = ln;   
}

CirAigGate::CirAigGate(unsigned vid, unsigned ln)
{
   _type = "AIG";
   _vId = vid;
   _lineno = ln;
}

CirPoGate::CirPoGate(unsigned vid, unsigned ln)
{
   _type = "PO";
   _vId = vid;
   _lineno = ln;
}

CirUndefGate::CirUndefGate(unsigned id)
{
   _type = "UNDEF";
   _vId = id;
}
 
CirConstGate::CirConstGate()
{
   _type = "CONST";
   _vId = 0;
}
