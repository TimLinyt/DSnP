/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <sstream>
#include <algorithm>
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
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
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

   m = miloa[0];
   i = miloa[1];
   l = miloa[2];
   o = miloa[3];
   a = miloa[4];   
   if (m < i+l+a) {
      errMsg = "Number of variables";
      errInt = m;
      return parseError(NUM_TOO_SMALL);
   }
   ++lineNo;

   _vidgates.resize(m+o+1);

   //PI
   for (size_t n = 0; n < i; ++n) {
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
      if (key > 2*m +1) {
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
   for (size_t n = 1; n <= o; ++n) {
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
      if (key > 2*m +1) {
         errInt = key;
         return parseError(MAX_LIT_ID);
      }
      colNo += e+1;
      if (e != string::npos) {
         return parseError(MISSING_NEWLINE);
      }
      ++lineNo;
      CirGate* ng = new CirPoGate(m+n, lineNo);
      _gates.push_back(ng);
      _vidgates[m+n] = ng; 
      _poArg.push_back(key);
   }
   //AIG
   vector< vector<unsigned> > _aigArg;
   for (size_t n = 0; n < a; ++n) {
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
         if (temp > 2*m +1) {
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
         for (size_t n = 0; n < token.size(); n++) {
            colNo++;
            if(!isprint(int(token[n]))) { 
               errInt = (int)token[n];
               return parseError(ILLEGAL_SYMBOL_NAME);
            }
         }
         if (line[0] == 'i') {
            if (key >= i) {
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
            if (key >= o) {
               errMsg = "PO index";
               errInt = key;
               return parseError(NUM_TOO_BIG);
            }
            if (_gates[key+i+l]->getSymbolStr() != "") {
               errMsg = "o";
               errInt = key;
               return parseError(REDEF_SYMBOLIC_NAME);
            }
            _gates[key+i+l]->setsymbol(token);
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
   //PO link
   for (size_t n = 0; n < o; n++) {
      linkFanio(m+n+1, _poArg[n]);
   }
   //Aig link
   for (size_t n = 0; n < a; n++) {
      unsigned _gid = _gates[i+l+o+n]->getId();
      linkFanio(_gid, _aigArg[n][0]);
      linkFanio(_gid, _aigArg[n][1]);
   }
   return true;
}

void 
CirMgr::linkFanio(const unsigned& gid, const unsigned& lid) 
{
   unsigned vid = lid/2;
   
   if (_vidgates[vid] == 0) {
      if (vid == 0) {
         _vidgates[0] = new CirConstGate();
      }
      else {
         _vidgates[vid] = new CirUndefGate(vid);
         _undef.push_back(vid);
      }
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
   cout << "Circuit Statistics" << endl;
   cout << "==================" << endl;
   cout << "  " << setw(7) << left << "PI" << setw(7) << right << i << endl;
   cout << "  " << setw(7) << left << "PO" << setw(7) << right << o << endl;
   cout << "  " << setw(7) << left << "AIG" << setw(7) << right << a << endl;
   cout << "------------------" << endl;
   cout << "  " << setw(7) << left << "Total" << setw(7) << right << i+o+a;
   cout << endl;
}

void
CirMgr::printNetlist() const
{
   IdList dsfList;
   CirGate::setGlobalRef();
   for (size_t n = 1; n <= o; n++) {
      _vidgates[m+n]->dfsTraversal(dsfList);
   }
   for (size_t n = 0; n < dsfList.size(); n++) {
      cout << "[" << n << "] ";
      _vidgates[dsfList[n]]->printGate();
      cout << endl; 
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (size_t n = 0; n < i; n++) {
      cout <<  ' ' << _gates[n]->getId();
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (size_t n = 0; n < o; n++) {
      cout << ' ' << m+n+1;
   }
   cout << endl;
}

void
CirMgr::printFloatGates()
{
   IdList _fl, _un;
   for (size_t n = 0; n < _undef.size(); n++) {
      IdList temp = ((CirUndefGate*)_vidgates[_undef[n]])->getFanout();
      for (size_t nn = 0; nn < temp.size(); nn ++) {
         _fl.push_back(temp[nn]);
      }
   }

   for (size_t n = 0; n < i; n++) {
      if (_gates[n]->unused()) {
         _un.push_back(_gates[n]->getId());
      }
   }
   for (size_t n = 0; n < a; n++) {
      if (_gates[i+l+o+n]->unused()) {
         _un.push_back(_gates[i+l+o+n]->getId());
      }
   }
   sort(_fl.begin(), _fl.end());
   sort(_un.begin(), _un.end());

   if (_fl.size()) {
      cout << "Gates with floating fanin(s):";
      for (size_t n = 0; n < _fl.size(); n++) {
         cout <<  ' ' << _fl[n];
      }  
      cout << endl;
   }
   if (_un.size()) {
      cout << "Gates defined but not used  :";
      for (size_t n = 0; n < _un.size(); n++) {
       cout <<  ' ' << _un[n];
      }
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   IdList dsfList, aiglist;
   CirGate::setGlobalRef();
   for (size_t n = 1; n <= o; n++) {
      _vidgates[m+n]->dfsTraversal(dsfList);
   }
   for (size_t n = 0; n < dsfList.size(); n++) {
      if (_vidgates[dsfList[n]]->getTypeStr() == "AIG") 
         aiglist.push_back(dsfList[n]);
   }
   outfile << "aag " << m << ' ' << i << ' ' << l << ' ' << o << ' ';
   outfile << aiglist.size() << endl;
   for (size_t n = 0; n < i; n++) {
      outfile << _gates[n]->getId()*2 << endl;
   }
   for (size_t n = 0; n < o; n++) {
      outfile << (_gates[i+l+n]->getFaninlist())[0] << endl;
   }
   for (size_t n = 0; n < aiglist.size(); n++) {
      outfile << aiglist[n]*2 << ' '; 
      outfile << (_vidgates[aiglist[n]]->getFaninlist())[0] << ' ';
      outfile << (_vidgates[aiglist[n]]->getFaninlist())[1] << endl;
   }
   for (size_t n = 0; n < i; n++) {
      if (_gates[n]->getSymbolStr() != "") {
         outfile << 'i' << n << ' ' << _gates[n]->getSymbolStr() << endl;
      }
   }
   for (size_t n = 0; n < o; n++) {
      if (_gates[i+l+n]->getSymbolStr() != "") {
         outfile << 'o' << n << ' ' << _gates[n]->getSymbolStr() << endl;
      }
   }
}



/**********************************************/

bool 
CirMgr::StrToUnsign(const string& token, unsigned& key) {
   if (token.size() == 0) return false;
   for (size_t n = 0; n < token.size(); n++) {
      if (token[n] > '9' || token[n] < '0') {
         return false;
      }
      key = (unsigned)(token[n] - '0') + 10 * key;

   }
   return true;
}
