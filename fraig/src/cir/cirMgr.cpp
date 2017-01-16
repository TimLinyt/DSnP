/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   lineNo = 0;
   
   fstream fin(fileName, ios::in);
   string line, token;
   unsigned miloa[5];
   if (!fin.is_open()) {
      cerr << "Cannot open design \"" << fileName << "\"!!";
      return false;
   }
   
   //aag M I L O A
   getline(fin, line, '\n');
   colNo = 0;
   //find aag 
   size_t e = line.find_first_of(' ');
   token = line.substr(0, e);  
   line = line.substr(e+1);

   
   if (token != "aag") {
      errMsg = token;
      return parseError(ILLEGAL_IDENTIFIER);
   }

   colNo += e+1;

   //find MILOA
   for (size_t count = 0; count < 5; count ++) {
      unsigned key = 0;
      if (e == string::npos) {
         errMsg = "number of ";
         if (count == 0) errMsg += "variables";
         else if (count == 1) errMsg += "PIs";
         else if (count == 2) errMsg += "latches";
         else if (count == 3) errMsg += "POs";
         else if (count == 4) errMsg += "AIGs";

         return parseError(MISSING_IDENTIFIER);
      }
      e = line.find_first_of(' ');
      token = line.substr(0, e);
      line = line.substr(e+1);
      if (!StrToUnsign(token, key)) {
         errMsg = "number of ";
         if (count == 0) errMsg += "variables";
         else if (count == 1) errMsg += "PIs";
         else if (count == 2) errMsg += "latches";
         else if (count == 3) errMsg += "POs";
         else if (count == 4) errMsg += "AIGs";
         errMsg = errMsg + "(" + token + ")";

         return parseError(ILLEGAL_NUM);
      }
      miloa[count] = key;
      colNo += e+1; 
   }

   if (e != string::npos) {
      return parseError(MISSING_NEWLINE);
   }

   _m = miloa[0];
   _i = miloa[1];
   _l = miloa[2];
   _o = miloa[3];
   _a = miloa[4];   
   if (_m < _i+_l+_a) {
      errMsg = "Number of variables";
      errInt = _m;
      return parseError(NUM_TOO_SMALL);
   }
   ++lineNo;

   _vidgates.resize(_m+_o+1);

   _vidgates[0] = new CirConstGate();
   //PI
   for (size_t n = 0; n < _i; ++n) {
      unsigned key = 0;
      getline(fin, line, '\n');
      if (fin.eof()) {
         errMsg = "PI";
         return parseError(MISSING_DEF);
      }
      colNo = 0;
      e = line.find_first_of(' ');

      if (!StrToUnsign(line, key)) {
         errMsg = "PI literal ID(" + line + ")";
         return parseError(ILLEGAL_NUM);
      }
      if (key/2 == 0) {
         errInt  = key;
         return parseError(REDEF_CONST);
      }
      if (key%2 == 1) {
         errMsg = "PI";
         errInt = key;
         return parseError(CANNOT_INVERTED);
      }
      if (key > 2*_m +1) {
         errInt = key;
         return parseError(MAX_LIT_ID);
      }
  
      if (_vidgates[key/2] != 0) {
         errInt = key;
         errGate = _vidgates[key/2];
         return parseError(REDEF_GATE);
      }
      colNo += e+1;
      if (e != string::npos) {
         return parseError(MISSING_NEWLINE);
      }
      ++lineNo;
      CirGate* ng = new CirPiGate(key/2, lineNo);
      _gates.push_back(ng);
      _vidgates[key/2] = ng;
   }

   //Latch
   //PO
   vector<unsigned> _poArg;
   for (size_t n = 1; n <= _o; ++n) {
      unsigned key = 0;
      getline(fin, line, '\n');
      if (fin.eof()) {
         errMsg = "PO";
         return parseError(MISSING_DEF);
      }
      colNo = 0;
      e = line.find_first_of(' ');

      if (line == "") {
         errMsg = "PI literal ID";
         return parseError(MISSING_IDENTIFIER);
      }
      if (!StrToUnsign(line, key)) {
         errMsg = "PO literal ID(" + line + ")";
         return parseError(ILLEGAL_NUM);
      }
      if (key > 2*_m +1) {
         errInt = key;
         return parseError(MAX_LIT_ID);
      }
      colNo += e+1;
      if (e != string::npos) {
         return parseError(MISSING_NEWLINE);
      }
      ++lineNo;
      CirGate* ng = new CirPoGate(_m+n, lineNo);
      _gates.push_back(ng);
      _vidgates[_m+n] = ng; 
      _poArg.push_back(key);
   }
   //AIG
   vector< vector<unsigned> > _aigArg;
   for (size_t n = 0; n < _a; ++n) {
      vector<unsigned> aaig;
      unsigned key;
      getline(fin, line, '\n');
      if (fin.eof()) {
         errMsg = "AIG";
         return parseError(MISSING_DEF);
      }
      colNo = 0;
      e = line.find_first_of(' ');

      for (size_t count = 0; count < 3; count ++) {
         unsigned temp = 0;
         if (e == string::npos) {
            errMsg = "AIG literal ID";
            return parseError(MISSING_IDENTIFIER);
         }
         e = line.find_first_of(' ');
         token = line.substr(0, e);
         line = line.substr(e+1);

         if (!StrToUnsign(token, temp)) {
            errMsg = "AIG literal ID(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
         if (temp%2 == 1 && count == 0) {
            errMsg = "AIG";
            errInt = temp;
            return parseError(CANNOT_INVERTED);
         }
         if (temp > 2*_m +1) {
            errInt = temp;
            return parseError(MAX_LIT_ID);
         }
         colNo += e+1;
         if (count == 0) key = temp;
         else aaig.push_back(temp);
      }
      if (e != string::npos) {
         return parseError(MISSING_NEWLINE);
      }

      if (key/2 == 0) {
         errInt  = key;
         return parseError(REDEF_CONST);
      }
      if (_vidgates[key/2] != 0) {
         errInt = key;
         errGate = _vidgates[key/2];
         return parseError(REDEF_GATE);
      }

      ++lineNo;
      CirGate* ng = new CirAigGate(key/2, lineNo);
      _gates.push_back(ng);
      _vidgates [key/2] = ng;
      _aigArg.push_back(aaig);
   }
   //Symbol
   
   getline(fin, line, '\n');
   if(!fin.eof()){
      if (line == "") {
         errMsg = "";
         return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      while(line != "") {
         colNo = 0;
         e = line.find_first_of(' ');
         if (line[0] == 'i' || line[0] == 'o') {
            unsigned key = 0;
            token = line.substr(1, e-1);
            if (token == "" || e == string::npos) {
               errMsg = "symbolic name";
               return parseError(MISSING_IDENTIFIER);
            }
            if (!StrToUnsign(token, key)) {
               errMsg = token;
               return parseError(ILLEGAL_IDENTIFIER);
            }
            token = line.substr(e+1, string::npos);
            if (token == "") {
               errMsg = "symbolic name";
               return parseError(MISSING_IDENTIFIER);
            }
            colNo += e;
            for (size_t n = 0, nn = token.size();  n < nn; n++) {
               colNo++;
               if(!isprint(int(token[n]))) { 
                  errInt = (int)token[n];
                  return parseError(ILLEGAL_SYMBOL_NAME);
               }
            }
            if (line[0] == 'i') {
               if (key >= _i) {
                  errMsg = "PI index";
                  errInt = key;
                  return parseError(NUM_TOO_BIG);
               }
               if (_gates[key]->getSymbolStr() != "") {
                  errMsg = "i";
                  errInt = key;
                  return parseError(REDEF_SYMBOLIC_NAME);
               }
               _gates[key]->setsymbol(token);
            }
            else {
               if (key >= _o) {
                  errMsg = "PO index";
                  errInt = key;
                  return parseError(NUM_TOO_BIG);
               }
               if (_gates[key+_i+_l]->getSymbolStr() != "") {
                  errMsg = "o";
                  errInt = key;
                  return parseError(REDEF_SYMBOLIC_NAME);
               }
               _gates[key+_i+_l]->setsymbol(token);
            }
         }
         //Comments
         else if (line[0] == 'c') {
            if (e != string::npos) {
               colNo += e+1;
               return parseError(MISSING_NEWLINE);
            }
            break;
         }
         else {
            errMsg = line[0];
            return parseError(ILLEGAL_SYMBOL_TYPE);
         }
         ++lineNo;
         getline(fin, line, '\n');
      }
   }
   //PO link
   for (size_t n = 0; n < _o; n++) {
      linkFanio(_m+n+1, _poArg[n]);
   }
   //Aig link
   for (size_t n = 0; n < _a; n++) {
      unsigned _gid = _gates[_i+_l+_o+n]->getId();
      linkFanio(_gid, _aigArg[n][0]);
      linkFanio(_gid, _aigArg[n][1]);
   } 
   
   updateDfsList();
   updateFloating();
   updateUnused();

   return true;
}

void 
CirMgr::linkFanio(const unsigned& gid, const unsigned& lid) 
{
   unsigned vid = lid/2;
   
   if (_vidgates[vid] == 0) {
      _vidgates[vid] = new CirUndefGate(vid);
   }
   _vidgates[gid]->setIn(CirGateV(_vidgates[vid], lid%2));
   _vidgates[vid]->setOut(CirGateV(_vidgates[gid], lid%2));
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{ 
   size_t piNum = 0, poNum = 0, aigNum = 0;

   for (size_t i = 1; i <= _m+_o; i++) {
      if (_vidgates[i]) {
         if (_vidgates[i]->getTypeStr() == "PI") piNum++;
         else if (_vidgates[i]->getTypeStr() == "PO") poNum++;
         else if (_vidgates[i]->getTypeStr() == "AIG") aigNum++;
      }
   }

   cout << endl;
   cout << "Circuit Statistics" << endl;
   cout << "==================" << endl;
   cout << "  " << setw(7) << left << "PI" << setw(7) << right << piNum << endl;
   cout << "  " << setw(7) << left << "PO" << setw(7) << right << poNum << endl;
   cout << "  " << setw(7) << left << "AIG" << setw(7) << right <<aigNum<< endl;
   cout << "------------------" << endl;
   cout << "  " << setw(7) << left << "Total";
   cout << setw(7) << right << piNum + poNum + aigNum;
   cout << endl;
}

void
CirMgr::updateDfsList()
{
   CirGate::setGlobalRef();
   _dfsList.clear();
   for (size_t n = 1; n <= _o; n++) {
      _vidgates[_m+n]->dfsTraversal(_dfsList);
   }

}

void
CirMgr::updateFloating()
{
   _floating.clear();
   for (size_t z = 1, n = _vidgates.size(); z < n; z++) {
      if (_vidgates[z]) {
         if (_vidgates[z]->isfloat()) {
            _floating.push_back(z);
         }
      }
   }
}

void
CirMgr::updateUnused()
{
   _unused.clear();
   for (size_t z = 1, n = _vidgates.size()-_o; z < n; z++) {
      if (_vidgates[z]) {
         if (_vidgates[z]->unused()) { 
            _unused.push_back(z);
         }
      }
   }
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _vidgates[_dfsList[i]]->printGate();
      cout << endl;
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (size_t n = 0; n < _i; n++) {
      cout <<  ' ' << _gates[n]->getId();
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (size_t n = 0; n < _o; n++) {
      cout << ' ' << _m+n+1;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   if (_floating.size()) {
      cout << "Gates with floating fanin(s):";
      for (size_t n = 0, nn = _floating.size(); n < nn; n++) {
         cout <<  ' ' << _floating[n];
      }  
      cout << endl;
   }
   if (_unused.size()) {
      cout << "Gates defined but not used  :";
      for (size_t n = 0, nn = _unused.size(); n < nn; n++) {
       cout <<  ' ' << _unused[n];
      }
      cout << endl;
   }
}

void
CirMgr::printFECPairs() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

/********************************************************/

bool 
CirMgr::StrToUnsign(const string& token, unsigned& key) {
   for (size_t n = 0, nn = token.size(); n < nn; n++) {
      if (token[n] > '9' || token[n] < '0') {

         return false;
      }
      key = (unsigned)(token[n] - '0') + 10 * key;

   }
   return true;
}
