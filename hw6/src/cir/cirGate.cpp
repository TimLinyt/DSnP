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
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
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

CirPoGate::CirPoGate(unsigned oid,unsigned ln)
{
   _type = "PO";
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
