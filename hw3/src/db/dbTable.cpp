/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include <sstream>
#include "dbTable.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   for (int i = 0; i < r.size(); i++) {
      if (r[i] == INT_MAX) os << '.';
      else os << r[i];
      if (i != r.size()-1) os << ' ';
      else cout << endl;
   }
   return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells are printed as '.'
   for (int i = 0; i < t.nRows(); i++) {
      for (int j = 0; j < t.nCols(); j++) {
         if (t[i][j] == INT_MAX)
            os << setw(6) << '.';
         else
            os << setw(6) << t[i][j];
      }
      os << endl;
   }
   return os;
}

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume all the data of the table are in a single line.
   while(!ifs.eof()) { 
      string sRow;
      DBRow Row;
      getline(ifs, sRow, '\r');
      if (sRow.empty()) break;
      stringstream iRow(sRow);
      while(!iRow.eof()) {
         string cell;
         int element;
         getline(iRow, cell, ',');
         if (!myStr2Int(cell, element)) 
            Row.addData(INT_MAX);
         else 
            Row.addData(element);
      }
      t.addRow(Row);
   }
   return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void
DBRow::removeCell(size_t c)
{
   // TODO
   for (int i = c; i < _data.size(); i++) {
      _data[i] = _data[i+1];
   }
   _data.pop_back();
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder
   for (int i = 0; i < _sortOrder.size(); i++) {
      if (r1[_sortOrder[i]] > r2[_sortOrder[i]]) return false;
      else if (r1[_sortOrder[i]] < r2[_sortOrder[i]]) return true;
   }
   return false;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void
DBTable::reset()
{
   // TODO
   vector<DBRow> tmp;
   for (int i = 0; i < nRows(); i++) {
      _table[i].reset();
   }
   tmp.swap(_table);
}

void
DBTable::addCol(const vector<int>& d)
{
   // TODO: add a column to the right of the table. Data are in 'd'.
   for (int i = 0; i < nRows(); i++) {
      _table[i].addData(d[i]);
   }
}

void
DBTable::delRow(int c)
{
   // TODO: delete row #c. Note #0 is the first row.
   _table[c].reset();
   for (int i = c; i < nRows(); i++) {
      _table[c] = _table[c+1];
   }
   _table.pop_back();
}

void
DBTable::delCol(int c)
{
   // delete col #c. Note #0 is the first row.
   for (size_t i = 0, n = _table.size(); i < n; ++i)
      _table[i].removeCell(c);
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in column #c are null, return NAN
// - Return "float" because NAN is a float.
float
DBTable::getMax(size_t c) const
{
   // TODO: get the max data in column #c
   int Max = INT_MIN;
   bool nocell = true;
   for (int i = 0; i < nRows(); i++) {
      if (_table[i][c] > Max && _table[i][c] != INT_MAX) {
         Max = _table[i][c];
         nocell = false;
      }
   }
   if (nocell) return NAN;
   else return Max;
}

float
DBTable::getMin(size_t c) const
{
   // TODO: get the min data in column #c
   int Min = INT_MAX;
   bool nocell = true;
   for (int i = 0; i < nRows(); i++) {
      if (_table[i][c] < Min && _table[i][c] != INT_MAX) {
         Min = _table[i][c];
         nocell = false;
      }
   }
   if (nocell) return NAN;
   else return (float)Min;
}

float 
DBTable::getSum(size_t c) const
{
   // TODO: compute the sum of data in column #c
   int Sum = 0;
   bool nocell = true;
   for (int i = 0; i < nRows(); i++) {
      if (_table[i][c] != INT_MAX) {
         Sum += _table[i][c];
         nocell = false;
      }
   }
   if (nocell) return NAN;
   else return (float)Sum;
}

int
DBTable::getCount(size_t c) const
{
   // TODO: compute the number of distinct data in column #c
   // - Ignore null cells
   int counter = 0;
   bool dData = true;
   for (int i = 0; i < nRows(); i++) {
      if (_table[i][c] != INT_MAX) {
         for (int j = i-1; j >= 0; j--) {
            if (_table[i][c] == _table[j][c]) {
               dData = false;
            }
         }
      if(dData) counter ++;
      }
   }
   return counter;
}

float
DBTable::getAve(size_t c) const
{
   // TODO: compute the average of data in column #c
   int counter = 0;
   float Sum = 0;
   for (int i = 0; i < nRows(); i++) {
      if(_table[i][c] != INT_MAX) {
         counter ++;
         Sum += _table[i][c];
      }
   }
   if(counter == 0) return NAN;
   else return Sum/counter;
   
}

void
DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
   std::sort(_table.begin(), _table.end(), s);
}

void
DBTable::printCol(size_t c) const
{
   // TODO: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   for (int i = 0; i < nRows(); i++) {
      if (_table[i][c] == INT_MAX) cout << '.';
      else cout << _table[i][c];
      if (i != nRows()-1) cout << ' ';
      else cout << endl;
   }
}

void
DBTable::printSummary() const
{
   size_t nr = nRows(), nc = nCols(), nv = 0;
   for (size_t i = 0; i < nr; ++i)
      for (size_t j = 0; j < nc; ++j)
         if (_table[i][j] != INT_MAX) ++nv;
   cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
        << nv << ")" << endl;
}

