/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () { _node = _node->_next; return *(this); }
      iterator operator ++ (int) { 
         iterator temp = *(this);
         _node = _node->_next;
         return temp; 
      }
      iterator& operator -- () { _node = _node->_prev; return *(this); }
      iterator operator -- (int) { 
         iterator temp = *(this);
         _node = _node->_prev;
         return temp;
      }

      iterator& operator = (const iterator& i) { 
         _node = i._node;
         return *(this); 
      }

      bool operator != (const iterator& i) const { 
         return (_node != i._node);
      }
      bool operator == (const iterator& i) const {
         return (_node == i._node);
      }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator(_head->_next); }
   iterator end() const { return iterator(_head); }
   bool empty() const { return (_head->_next == _head); }
   size_t size() const {
      size_t _size = 0;
      for (iterator it = begin(); it != end(); it++) _size++;
      return _size; 
   }

   void push_back(const T& x) {
      DListNode<T>* temp = new DListNode<T>(x, _head->_prev, _head);
      temp->_prev->_next = temp;
      _head->_prev = temp;
   }
   void pop_front() { erase(begin()); }
   void pop_back() { erase(--end()); }

   // return false if nothing to erase
   bool erase(iterator pos) { 
      if (empty()) return false;
      pos._node->_next->_prev = pos._node->_prev;
      pos._node->_prev->_next = pos._node->_next;
      delete pos._node;
      return true;
   }
   bool erase(const T& x) { 
      for (iterator it = begin(); it != end(); it++) {
         if (*it == x) return erase(it); 
      }
      return false; 
   }
   void clear() { while(!empty()) pop_front(); }  // delete all nodes except for the dummy node

   void sort() const { 
      quick_sort(begin()._node, end()._node->_prev);
   }

private:
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions

   /*void bubble() {
      bool flag = true;
      while(flag) {
         flag = false;
         for (iterator i = begin(); i != end(); ) {
            if (*i > *(++i) && i != end()) {
               DListNode<T>* temp = i._node->_prev;
               i._node->_next->_prev = temp;
               temp->_prev->_next = i._node;
               i._node->_prev = temp->_prev;
               temp->_prev = i._node;
               temp->_next = i._node->_next;
               i._node->_next = temp;
               flag = true;
            }
         }
      }
   }*/

   DListNode<T>* partition(DListNode<T>* p, DListNode<T>* r) const {
      if (p == r) return p;
      T key = p->_data;
      bool flag = false;
      p = p->_prev;
      r = r->_next;
   while (1) {
         flag = false;
         while(1) {
            p = p->_next;
            if (p->_data >= key) break;  
         }
         while(1) {
            if(r == p) flag = true;
            r = r->_prev;
            if(r == p) flag = true;
            if (r->_data <= key) break;
         }
        
         if (!flag) {
            T tempt = p->_data;
            p->_data = r->_data;
            r->_data = tempt;
            /*DListNode<T>* temp = i._node->_prev;
            i._node->_prev = j._node->_prev;
            i._node->_prev->_next = i._node;
            j._node->_prev = temp;
            j._node->_prev->_next = j._node;
            temp = i._node->_next;
            i._node->_next = j._node->_next;
            i._node->_next->_prev = i._node;
            j._node->_next = temp;
            j._node->_next->_prev = j._node;
            
            temp = i._node;
            i._node = j._node;
            j._node = temp;
            */
         }
         else {
            /*p._node = temp_b._node->_next;
            r._node = temp_e._node->_prev;
            */
            return r;
         }
      }
   }

   void quick_sort(DListNode<T>* p, DListNode<T>* r) const {
      if (p == r) return;
      DListNode<T>* q = partition(p, r);
      quick_sort(p, q);
      quick_sort(q->_next, r);
   }
};

#endif // DLIST_H
