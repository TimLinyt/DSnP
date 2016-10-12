#ifndef P2_TABLE_H
#define P2_TABLE_H

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Row
{
public:
   const int operator[] (size_t i) const {return _data[i];} // TODO
   int& operator[] (size_t i) {return _data[i];} // TODO
   void del_Data() {delete [] _data;}
   Row(const int&);
   //~Row() {delete [] _data;}
private:
   int  *_data;
};

class Table
{
public:
   const Row& operator[] (size_t i) const {return _rows[i];};
   Row& operator[] (size_t i) {return _rows[i];};

   int get_cNumber() {return cNumber;};
   bool argument_check(int&);
   bool read(const string&);

   void print();
   void sum();
   void ave();
   void max();
   void min();
   void count();
   void add();
   void exit();
private:
   void cal_cNumber(istream&);
   int rNumber, cNumber;
   vector<Row>  _rows;
};

#endif // P2_TABLE_H
