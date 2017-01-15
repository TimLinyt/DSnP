/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class CirGateV;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGateV {
public:
   #define NEG 0x1
   CirGateV(CirGate* g, size_t phase): 
      _gateV(size_t(g) + phase) { }
   CirGate* gate() const {
      return (CirGate*)(_gateV & ~size_t(NEG)); }
   bool isInv() const { return (_gateV & NEG); }
private:
   size_t           _gateV;
};

class CirGate
{
public:
   CirGate(){}
   virtual ~CirGate() {}
   // Basic access methods
   string getTypeStr() const { return _type; }
   string getSymbolStr() const { return _symbol; }
   unsigned getLineNo() const { return _lineno; }
   virtual bool isAig() const { return false; }

   // Printing functions
   virtual void printGate() const {}
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);
   
   //my
   unsigned getId() const { return _vId; } 
   IdList getFaninlist() const;
   IdList getFanoutlist() const;
   bool unused() const { return !(_type == "PO" || _out.size()); }
   bool isfloat();
   

  //dfs
   static unsigned _globalRef;
   unsigned        _ref;
   bool isGlobalRef()  const { return ( _ref == _globalRef); }
   void setToGlobalRef() { _ref = _globalRef; }
   static void setGlobalRef() { _globalRef++; }
   void dfsTraversal(IdList& dfsList);
   void Fanin(int level, int nowlevel, bool isInv);
   void Fanout(int level, int nowlevel, bool isInv);

   // Setting functions_in
   void setIn(CirGateV ng) { _in.push_back(ng); }
   void setOut(CirGateV ng) { _out.push_back(ng); }
   void deleteOut(CirGate* g);
   void replacein(CirGateV oldg, CirGateV newg);
   void setsymbol(const string& s) { _symbol = s; }

private:

protected:
   string _type, _symbol;
   unsigned _vId, _lineno;
   vector<CirGateV>  _in, _out;
};

class CirPiGate : public CirGate
{
public:
   CirPiGate(unsigned, unsigned);
   ~CirPiGate() {};

   void printGate() const;
};

class CirAigGate : public CirGate
{
public:
   CirAigGate(unsigned, unsigned);
   ~CirAigGate() {};
   
   bool isAig() const { return true; }
   void printGate() const;

};

class CirPoGate : public CirGate
{
public:
   CirPoGate(unsigned, unsigned);
   ~CirPoGate() {};
   
   void printGate() const;
};

class CirUndefGate : public CirGate
{
public:
   CirUndefGate(unsigned);
   ~CirUndefGate() {};

   void printGate() const;
};

class CirConstGate : public CirGate
{
public:
   CirConstGate();
   ~CirConstGate() {};
   
   void printGate() const;
};
#endif // CIR_GATE_H
