1v11                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }   
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   const Data& min() const { return _data[0]; }
   void insert(const Data& d) { 
      _data.push_back(d);
      size_t x = _data.size();
      while (x > 1 && d < _data[x/2 - 1]) { 
         _data[x - 1] = _data[x/2 - 1];
         x = x/2;
      }
      _data[x-1] = d;
   }
   void delMin() { delData(0); }
   void delData(size_t i) { 
      Data temp = _data[_data.size()-1];
      _data.resize(_data.size()-1);

      if (i == _data.size()) return;
      
      int p = i+1, t = 2 * p;
      while (t <= _data.size()) {
         if (t < _data.size()) // has right child
            if (_data[t] < _data[t-1])
               ++t; // to the smaller child 
         if (temp < _data[t-1])
            break;
         _data[p-1] = _data[t-1]; 
         p = t;
         t = 2 * p;
      }

      t = p / 2;
      while (t > 1) {
         if (_data[t-1] < temp)
            break;
         _data[p-1] = _data[t-1];
         p = t;
         t = p / 2;
      }
      
      _data[p-1] = temp;
   }

private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
