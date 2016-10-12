#include <iostream>
#include <string>
#include <sstream>
#include "p2Table.h"

using namespace std;

int main()
{
   Table table;

   // TODO: read in the csv file
   string csvFile;
   cout << "Please enter the file name: ";
   cin >> csvFile;
   //csvFile = "test_hw1.csv";
   if (table.read(csvFile))
      cout << "File \"" << csvFile << "\" was read in successfully." << endl;
   else exit(-1); // csvFile does not exist.
   
   //test
   // TODO read and execute commands
   while (true) 
   {
      string commands;
      cin >> commands;
      
      if (commands == "PRINT") table.print();
      else if (commands == "SUM") table.sum();
      else if (commands == "AVE") table.ave();
      else if (commands == "MAX") table.max();
      else if (commands == "MIN") table.min();
      else if (commands == "COUNT") table.count();
      else if (commands == "ADD") table.add();
      else if (commands == "EXIT") 
      {
         table.exit();
         break;
      }
      else
         cout << "Commamd is invalid." << endl;

      cin.clear();
      cin.ignore(INT_MAX, '\n');
   }  
}
