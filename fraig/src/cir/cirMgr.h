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

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const{return (gid <= _m+_o)?_vidgates[gid]:0;}

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   ofstream           *_simLog;
   GateList _gates, _vidgates;
   vector<GateList> _fecGrps;
   unsigned _m, _i, _l, _o, _a;
   IdList _dfsList, _floating, _unused;
   

   bool StrToUnsign(const string&, unsigned&); 
   void linkFanio(const unsigned& gid, const unsigned& lid);
   void updateDfsList();
   void updateFloating();
   void updateUnused();
  
   void rnGenSim();
   void UpdateFec();
   unsigned MaxFail();
   void fecInit();
   void identifyFEC();
   void simulate(); 
   void ptnToSim(vector<string>& ptns, size_t count = 64); 
   void writeLog(vector<string>& ptns);
   bool checkerr(string & str); 

   //sat
   void sortFecGrps();
   void InitSAT(SatSolver& solver);
   bool proofFECpair(SatSolver& solver, CirGate* g1, CirGate* g2);
   void MergeGate(CirGate* master, CirGate* slave);   
   void fraigMerge(CirGate* master, CirGate* slave, bool inv);
   void getSatAssignment(SatSolver& solver, vector<size_t>& ptn, int& count);
   void deleteinvaildFEC();


};

#endif // CIR_MGR_H
