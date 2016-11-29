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

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
unsigned CirGate::_globalRef = 1;

void 
CirGate::dfsTraversal(IdList& dfsList) 
{
   for (size_t n = 0; n < _in.size(); n++) {
      if (!_in[n]->isGlobalRef()) {
         _in[n]->setToGlobalRef();
         _in[n]->dfsTraversal(dfsList);
      }
   }
   dfsList.push_back(_vId);
}

void
CirGate::Fanin(int level, int nowlevel) 
{
   cout << _type << ' ' << _vId;
   if (level > nowlevel && _in.size() > 0) {
      if (isGlobalRef()) {
         cout << " (*)";
         cout << endl;
      }
      else {
         cout << endl;
         for (size_t n = 0; n < _in.size(); n++) {
            for (size_t nn = 0; nn < nowlevel+1; nn++) { cout << "  "; }
            if (_type == "PO") {
               if (((CirPoGate*)this)->getFanin()%2 == 1) cout << '!';
            }
            else if (_type == "AIG") {
               unsigned* ii = ((CirAigGate*)this)->getFanin();
               if (ii[n]%2 == 1) cout << '!';
            }
            _in[n]->Fanin(level, nowlevel+1);
         }
      }
   }
   else cout << endl;
   CirGate::setToGlobalRef();
}
void
CirGate::Fanout(int level, int nowlevel) 
{
   cout << _type << ' ' << _vId;
   if (level > nowlevel && _out.size() > 0) {
      if (isGlobalRef()) {
         cout << " (*)";
         cout << endl;
      }
      else {
         cout << endl;
         for (size_t n = 0; n < _out.size(); n++) {
            for (size_t nn = 0; nn < nowlevel+1; nn++) { cout << "  "; }
            if (_out[n]->getTypeStr() == "PO") {
               if (((CirPoGate*)_out[n])->getFanin()%2 == 1) cout << '!';
            }
            else if (_out[n]->getTypeStr() == "AIG") {
               unsigned* ii = ((CirAigGate*)_out[n])->getFanin();
               if (ii[0]/2 == ii[1]/2 && ii[0]%2 != ii[1]%2) {
                  if (!n && ii[0]%2) cout << '!';
               }
               else if (ii[0]%2 == 1 && ii[0]/2 == _vId) cout << '!';
               else if( ii[1]%2 == 1 && ii[1]/2 == _vId) cout << '!';
            }
            _out[n]->Fanout(level, nowlevel+1);
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
   Fanin(level, 0);
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   setGlobalRef();
   Fanout(level, 0);
}

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
      if (_in[n]->getTypeStr() == "UNDEF") cout << '*';
      if (_i[n]%2 == 1) cout << '!';
      cout << _i[n]/2;
   }
}


void 
CirPoGate::printGate() const 
{
   cout << "PO  " << _vId << ' ';
   if (_in[0]->getTypeStr() == "UNDEF") cout << '*';
   if (_i%2 == 1) cout << '!';
   cout << _i/2;
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

CirPiGate::CirPiGate(unsigned lid, unsigned ln)
{
   _type = "PI";
   _vId = lid/2;
   _lineno = ln;   
}

CirAigGate::CirAigGate(unsigned* abc, unsigned ln)
{
   _type = "AIG";
   _vId = abc[0]/2;
   _lineno = ln;
   _i = new unsigned[2];
   _i[0] = abc[1];
   _i[1] = abc[2];
}

CirPoGate::CirPoGate(unsigned oid,unsigned vid, unsigned ln)
{
   _type = "PO";
   _vId = vid;
   _lineno = ln;
   _i = oid;
}

CirUndefGate::CirUndefGate(unsigned id)
{
   _type = "UNDEF";
   _vId = id;
}
 
IdList
CirUndefGate::getFanout()
{
   IdList temp;
   for (size_t n = 0; n < _out.size(); n ++) 
      temp.push_back(_out[n]->getId());
   return temp;
}

CirConstGate::CirConstGate()
{
   _type = "CONST";
   _vId = 0;
}
