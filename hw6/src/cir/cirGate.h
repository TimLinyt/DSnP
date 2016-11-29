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

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
   CirGate() :_ref(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return _type; }
   string getSymbolStr() const { return _symbol; }
   unsigned getId() const { return _vId; } 
   unsigned getLineNo() const { return _lineno; }
   bool unused() const { return !(_out.size()); }

   //dfs
   static unsigned _globalRef;
   unsigned        _ref;
   bool isGlobalRef()  const { return ( _ref == _globalRef); }
   void setToGlobalRef() { _ref = _globalRef; }
   static void setGlobalRef() { _globalRef++; }
   void dfsTraversal(IdList& dfsList);
   void Fanin(int level, int nowlevel);
   void Fanout(int level, int nowlevel);

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);

   // Setting functions_in
   void setIn(CirGate* ng) { _in.push_back(ng); }
   void setOut(CirGate* ng) { _out.push_back(ng); }
   void setsymbol(const string& s) { _symbol = s; }

private:

protected:
   string _type, _symbol;
   unsigned _vId, _lineno;
   GateList _in, _out;
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
   CirAigGate(unsigned*, unsigned);
   ~CirAigGate() {};

   void printGate() const;
   unsigned* getFanin() { return _i; }

private:
  unsigned* _i;

};

class CirPoGate : public CirGate
{
public:
   CirPoGate(unsigned, unsigned, unsigned);
   ~CirPoGate() {};
   
   void printGate() const;
   unsigned getFanin() { return _i; }

private:
   unsigned _i;

};

class CirUndefGate : public CirGate
{
public:
   CirUndefGate(unsigned);
   ~CirUndefGate() {};

   void printGate() const;
   IdList getFanout();
};

class CirConstGate : public CirGate
{
public:
   CirConstGate();
   ~CirConstGate() {};
   
   void printGate() const;
};

#endif // CIR_GATE_H
