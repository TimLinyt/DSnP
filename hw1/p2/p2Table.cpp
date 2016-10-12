#include "p2Table.h"
#include <fstream>
#include <vector>
#include <iomanip>
#define EMPTY INT_MAX

using namespace std;

// Implement member functions of class Row and Table here
Row::Row (const int& ncol)
{
   _data = new int[ncol];
   for (int i = 0; i < ncol; i++) _data[i] = EMPTY;
}

bool Table::read(const string& csvFile)
{
   ifstream fin(csvFile);
   if(!fin) return false;
   cal_cNumber(fin);

   //put data in table
   rNumber = 0;
   while(!fin.eof())
   {
      Row row_temp(cNumber);
      int col = 0;
      while(!fin.eof())
      {
         if (fin.peek() == ',')
         {
            fin.ignore();
            col++;
         }
         else if (fin.peek() == '\r')
         {
            fin.ignore();
            while (fin.peek() == '\r') fin.ignore();
            _rows.push_back(row_temp);
            rNumber++;
            break;
         }
         else
         {
            fin >> row_temp[col];
         }
      } 
   }
   
   fin.close();
   return true; 
}

void Table::cal_cNumber(istream& is)
{
   int counter = 1;
   is.seekg(0);
   while(is.peek() != '\r')
   {
      if(is.peek() == ',')
      {
         counter++;
         is.ignore();
      }
      else is.ignore();
   }
   is.seekg(0);
   cNumber = counter;
}

void Table::print()
{
   for(int i = 0; i < rNumber; i++)
   {
      for(int j = 0; j < cNumber; j++)
      {
         if(_rows[i][j] == EMPTY) cout << setw(4) << right << " ";
         else  cout << setw(4) << right << _rows[i][j];
      }
      cout << endl;
   }
}

void Table::sum()
{
   int sum = 0, j;
   cin >> j;
   if(argument_check(j))
   {
      for (int i = 0; i < rNumber; i++)
      {
         if (_rows[i][j] != EMPTY) sum += _rows[i][j];
      }
      cout << "The summation of data in column #" << j << " is " << sum << "." << endl;
   }
}
   
void Table::ave()
{
   int sum = 0, counter = 0, j;
   cin >> j;
   if(argument_check(j))
   {
      for (int i = 0; i < rNumber; i++)
      {
         if (_rows[i][j] != EMPTY) 
         { 
            sum += _rows[i][j];
            counter ++;
         }
      }  
      cout << "The average of data in column #" << j << " is " ;
      cout << fixed << setprecision(1) << (double)sum/counter << "." << endl;
   }
}

void Table::max()
{
   int max = -EMPTY, j;
   cin >> j;
   if(argument_check(j))
   {
      for (int i = 0; i < rNumber; i++)
      {
         if (_rows[i][j] != EMPTY && _rows[i][j] > max)
            max = _rows[i][j];
      }
      cout << "The maximum of data in column #" << j << " is " << max << "." << endl;
   }
}

void Table::min()
{
   int min = EMPTY, j;
   cin >> j;   
   if(argument_check(j))
   {
      for (int i = 0; i < rNumber; i++)
      {
         if (_rows[i][j] != EMPTY && _rows[i][j] < min)
            min = _rows[i][j];
      }
      cout << "The minmum of data in column #" << j << " is " << min << "." << endl;
   }
}

void Table::count()
{
   int count = 0, j;
   cin >> j;
   if(argument_check(j)) 
   {
      for (int i = 0; i < rNumber; i++)
      {
         if(_rows[i][j] != EMPTY)
         {
            int distinct = 1;
            for (int k = i-1 ; k >= 0; k--)
            {
               if(_rows[i][j] == _rows[k][j]) 
               {
                  distinct = 0;
                  break;
               }
            }
            count += distinct;
         }
      }
      cout << "The distinct count of data in column #" << j << " is " << count << "." << endl;
   }
}

void Table::add()
{
   Row row_add(cNumber);
   int counter = 0;
   for (; counter < cNumber;)
   {
      if (cin.peek() == '\n')
         break;
      else if (cin.peek() == '-')
      {
         cin.ignore();
         if (cin.peek() == ' '|| cin.peek() == '\n') 
         {
            if(!cin.eof()) cin.ignore();
            row_add[counter] = EMPTY;
            counter++;
         }
         else 
         {
            cin >> row_add[counter];
            row_add[counter] *= -1;
            counter++;
         }
      }
      else
      {
         cin >> row_add[counter];
         counter ++;
      }
      while(cin.peek() == ' ') cin.ignore();
   }
   if(counter != cNumber || cin.peek() != '\n')
   {
      cout << "Argument is invalid." << endl;
   }
   else
   {
      _rows.push_back(row_add);
      rNumber++;
   }
}

bool Table::argument_check(int& j)
{
   if (j < 0 || j >= cNumber) 
   {
      cout << "Argumemt is invalid." << endl;
      return false;
   }
   while(cin.peek() == ' ') 
   {
      cin.ignore();
      if (cin.peek() != '\n') 
      {
         cout << "Argumemt is invalid." << endl;
         return false;
      }
   }
   return true;
}

void Table::exit()
{
   for (int i = 0; i < cNumber; i++)
   {
      _rows[i].del_Data();
   }
}

