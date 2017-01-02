/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() { 
      for (size_t n = 0; n < _vidgates.size(); n++) {
         if (_vidgates[n]) delete _vidgates[n];
      }
      _gates.clear();
      _vidgates.clear();
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { return _vidgates[gid]; }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates();
   void writeAag(ostream&) const;

private:
   GateList _gates, _vidgates;
   unsigned m, i, l, o, a;
   IdList _undef;

   bool StrToUnsign(const string&, unsigned&); 
   void linkFanio(const unsigned& gid, const unsigned& lid);
};

#endif // CIR_MGR_H
