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
   fstream fin(fileName, ios::in);
   string line, token;
   unsigned miloa[5];

   if (!fin.is_open()) {

      return false;
   }
   
   //aag M I L O A
   getline(fin, line, '\n');
   ++lineNo;
   //find aag 
   size_t e = line.find_first_of(' ');
   token = line.substr(0, e);  
   line = line.substr(e+1);

   if (token != "aag") {
      
      return false;
   }

   //find MILOA
   for (size_t count = 0; count < 5; count ++) {
      unsigned key = 0;
      if (e == string::npos) {

         return false;
      }
      e = line.find_first_of(' ');
      token = line.substr(0, e);
      line = line.substr(e+1);
      if (!StrToUnsign(token, key)) {

         return false;
      }
      miloa[count] = key;
      
   }

   if (e != string::npos) {

      return false;
   }
   m = miloa[0];
   i = miloa[1];
   l = miloa[2];
   o = miloa[3];
   a = miloa[4];   
   
   _vidgates.resize(m+o+1);

   //PI
   for (size_t n = 0; n < i; ++n) {
      unsigned key = 0;
      getline(fin, line, '\n');
      ++lineNo;
      if (!StrToUnsign(line, key)) {

         return false;
      }
      CirGate* ng = new CirPiGate(key, lineNo);
      _gates.push_back(ng);
      _vidgates[key/2] = ng;
      _pi.push_back(key/2);
   }

   //Latch
   
   //PO
   for (size_t n = 1; n <= o; ++n) {
      unsigned key = 0;
      getline(fin, line, '\n');
      ++lineNo;
      if (!StrToUnsign(line, key)) {

         return false;
      }
      CirGate* ng = new CirPoGate(key, lineNo);
      _gates.push_back(ng);
      _vidgates[m+n] = ng; 
   }

   //AIG
   for (size_t n = 0; n < a; ++n) {
      unsigned* abc = new unsigned[3];
      getline(fin, line, '\n');
      ++lineNo;
      e = line.find_first_of(' ');

      for (size_t count = 0; count < 3; count ++) {
         unsigned key = 0;
         if (e == string::npos) {

            return false;
         }
         e = line.find_first_of(' ');
         token = line.substr(0, e);
         line = line.substr(e+1);
         if (!StrToUnsign(token, key)) {
   
            return false;
         }
         abc[count] = key;
      }
      if (e != string::npos) {
         
         return false;
      }
      CirGate* ng = new CirAigGate(abc, lineNo);
      _gates.push_back(ng);
      _vidgates [abc[0]/2] = ng;
   }
   //Symbol
   getline(fin, line, '\n');
   while(line != "") {
      ++lineNo;
      e = line.find_first_of(' ');
      if (line[0] == 'i' || line[0] == 'o') {
         unsigned key = 0;
         token = line.substr(1, e);
         if (!StrToUnsign(token, key)) {
            
            return false;
         }
         token = line.substr(e+1, string::npos);
         if (line[0] == 'i') 
            _gates[key]->setsymbol(token);
         else
            _gates[key+i+l]->setsymbol(token);
      }
      //Comments
      else if (line[0] == 'c') {
         if (e != string::npos) {
            
            return false;
         }
         break;
      }
      else return false;
      getline(fin, line, '\n');
   }
   //PO link
   for (size_t n = 1; n <= o; n++) {
      unsigned _i = ((CirPoGate*)_vidgates[m+n])->getFanin();
      linkFanio(m+n, _i);
   }
   //Aig link
   for (size_t n = 0; n < a; n++) {
      unsigned _gid = _gates[i+l+o+n]->getId();
      unsigned* _i = ((CirAigGate*)_vidgates[_gid])->getFanin();
      linkFanio(_gid, _i[0]);
      linkFanio(_gid, _i[1]);
   }
   return true;
}

void 
CirMgr::linkFanio(const unsigned& gid, const unsigned& lid) 
{
   unsigned vid = lid/2;
   bool inv = lid%2;
   
   if (_vidgates[vid] == 0) {
      if (vid == 0) {
         _vidgates[0] = new CirConstGate();

      }
      else {
         _vidgates[vid] = new CirUndefGate(vid);
         _ud.push_back(vid);
      }
   }
   _vidgates[gid]->setIn(_vidgates[vid], inv);
   _vidgates[vid]->setOut(_vidgates[gid]);
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

}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (size_t n = 0; n < i; n++) {
      cout <<  ' ' << _pi[n];
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (size_t n = 0; n < o; n++) {
      //cout <<  ' ' << _po[n];
      cout << ' ' << m+n+1;
   }
   cout << endl;
}

void
CirMgr::printFloatGates()
{
   if (!_check_fl_un) {
      for (size_t n = 0; n < _ud.size(); n++) {
         IdList temp = ((CirUndefGate*)_vidgates[_ud[n]])->getFanout();
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
      _check_fl_un = true;
   }

   cout << "Gates with floating fanin(s):";
   for (size_t n = 0; n < _fl.size(); n++) {
      cout <<  ' ' << _fl[n];
   }  
   cout << endl;

   cout << "Gates defined but not used  :";
   for (size_t n = 0; n < _un.size(); n++) {
      cout <<  ' ' << _un[n];
   }

}

void
CirMgr::writeAag(ostream& outfile) const
{
}


/**********************************************/

bool 
CirMgr::StrToUnsign(const string& token, unsigned& key) {
   for (size_t n = 0; n < token.size(); n++) {
      if (token[n] > '9' || token[n] < '0') {

cout << "fuck u";
         return false;
      }
      key = (unsigned)(token[n] - '0') + 10 * key;

   }
   return true;
}
