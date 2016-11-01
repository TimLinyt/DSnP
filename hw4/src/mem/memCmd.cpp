/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   if(!CmdExec::lexOptions(option, tokens))
      return CMD_EXEC_ERROR;
   
   if (tokens.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");
   
   int n = 0, s = 0;
   for (size_t i = 0; i < tokens.size(); i++) {
      if (myStrNCmp("-Array", tokens[i], 2) == 0) {
         if(s != 0)
            return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
         if (++i >= tokens.size())
            return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i-1]);
         if(!myStr2Int(tokens[i], s) || s <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
      }
      else {
         if (n != 0)
            return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
         if (!myStr2Int(tokens[i], n) || n <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
      }
   }

   if (s == 0) mtest.newObjs(n);
   else mtest.newArrs(n, s);
   
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   if(!CmdExec::lexOptions(option, tokens))
      return CMD_EXEC_ERROR;
   
   if (tokens.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   bool doA = 0, doR = 0;
   int idx = -1, tIdx = -1;
   for(size_t i = 0; i < tokens.size(); i++) {
      if (myStrNCmp("-Index", tokens[i], 2) == 0) {
         if (idx >= 0) 
            return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
         if (++i >= tokens.size())
            return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i-1]);
         if (!myStr2Int(tokens[i], idx) || idx < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
         tIdx = i;
      }
      else if (myStrNCmp("-Random", tokens[i], 2) == 0) {
         if (idx >= 0) 
            return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
         if (++i >= tokens.size())
            return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i-1]);
         if (!myStr2Int(tokens[i], idx) || idx < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
         doR = true;
         tIdx = i;
      }
      else if (myStrNCmp("-Array", tokens[i], 2) == 0) {
         if (doA) 
            return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
         doA = true;
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
   }
   if (idx < 0)
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   if (doR) {
      if (doA) {
         if (mtest.getArrListSize() == 0) {
            cerr << "Size of array list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[tIdx]);
         }
         for (size_t i = 0; i < idx; i++) {
            mtest.deleteArr(rnGen(mtest.getArrListSize()));
         }
      }
      else {
         if (mtest.getObjListSize() == 0) {
            cerr << "Size of object list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[tIdx]);
         }
         for (size_t i = 0; i < idx; i++) {
            mtest.deleteObj(rnGen(mtest.getObjListSize()));
         }
      }
   }
   else {
      if (doA) {
         if (mtest.getArrListSize() <= idx) {
            cerr << "Size of array list (" << mtest.getArrListSize() <<
               ") is <= " << idx << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[tIdx]);
         }
         mtest.deleteArr(idx);
      }
      else {
         if (mtest.getObjListSize() <= idx) {
            cerr << "Size of object list (" << mtest.getObjListSize() <<
               ") is <= " << idx << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[tIdx]);
         }
         mtest.deleteObj(idx);
      }
   }

   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


