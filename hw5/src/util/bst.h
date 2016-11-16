/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;

   BSTreeNode(const T& d, BSTreeNode<T>* p = 0, BSTreeNode<T>* l = 0, 
              BSTreeNode<T>* r = 0):
      _data(d), _parent(p), _left(l), _right(r) {}

   T                 _data;
   BSTreeNode<T>*    _parent;
   BSTreeNode<T>*    _left;
   BSTreeNode<T>*    _right;
};


template <class T>
class BSTree
{
   // TODO: design your own class!!
public:
   BSTree() {
      _Dm  = new BSTreeNode<T>(T());
      _size = 0;
   }
   ~BSTree() { clear(); delete _Dm; delete _root;}

   class iterator 
   { 
      friend class BSTree;

   public:
      iterator(BSTreeNode<T>* n = 0): _node(n) {}
      iterator(const iterator& i)   : _node(i._node) {}
      ~iterator() {}

      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () {
         if (_node->_right != 0) {
            _node = _node->_right;
            while (_node->_left) _node = _node->_left;
         }
         else {
            while(1) {
               if(_node->_parent == 0) break;
               if(_node == _node->_parent->_left) {
                  _node = _node->_parent; 
                  break;
               }
               _node = _node->_parent;
            } 
            /*while (_node != _node->_parent->_left && _node->_parent != 0) {
               _node = _node->_parent;
            }*/
         }
         return *this;
      }
      iterator operator ++ (int) { 
         iterator temp = *this;
         ++(*this);
         return temp; 
      }
      iterator& operator -- () { 
         if (_node->_left != 0) {
            _node = _node->_left;
            while (_node->_right) _node = _node->_right;
         }
         else  {
            while(1) {
               if(_node->_parent == 0) break;
               if(_node == _node->_parent->_right) {
                  _node = _node->_parent; 
                  break;
               }
               _node = _node->_parent;
            } 
            /*while (_node != _node->_parent->_right && _node->_parent != 0) {
               _node = _node->_parent;
            }*/
         }
         return *this; 
      }
      iterator operator -- (int) { 
         iterator temp = *this;
         --(*this);
         return temp; 
      }
      iterator & operator = (const iterator& i) { 
         _node = i._node; return *this; 
      }
      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

   private:
      BSTreeNode<T>* _node;
   };

   iterator begin() const {iterator temp(_Dm); if (_size) ++temp; return temp; }
   iterator end() const { return iterator(_Dm); }
   bool empty() const { return !(_size); }
   size_t size() const { return _size; }

   void insert (const T& x) {
      if (empty()) {
         _root = new BSTreeNode<T>(x, _Dm);
         _Dm->_left = _root;
         _Dm->_right = _root;
      }
      else {
         BSTreeNode<T>* temp = _root;
         BSTreeNode<T>* temp_p = _root;
         while(temp) {
            temp_p = temp;
            if (x < temp->_data) temp = temp->_left;
            else temp = temp->_right;
         }
         if (x < temp_p->_data) temp_p->_left = new BSTreeNode<T>(x, temp_p);
         else temp_p->_right = new BSTreeNode<T>(x, temp_p);
      }
      _size ++;
   }
   void pop_front() {erase(begin());}
   void pop_back() {erase(--end());}

   void sort() {}
   void print() {}

   bool erase(iterator pos) { 
      if (empty()) return false;
      BSTreeNode<T>* temp = pos._node;
      if (temp->_left == 0 && temp->_right == 0) {
         if (temp == temp->_parent->_left) 
            temp->_parent->_left = 0;
         else 
            temp->_parent->_right = 0;
      }
      else if (temp->_left == 0) {
         Transplant(temp, temp->_right);
      }
      else if (temp->_right == 0) {
         Transplant(temp, temp->_left);
      }
      else {
         ++pos;
         if (pos._node->_parent != temp) {
            if (pos._node->_right != 0) Transplant(pos._node, pos._node->_right);
            else pos._node->_parent->_left = 0;
            pos._node->_right = temp->_right;
            pos._node->_right->_parent = pos._node;
         }
         Transplant(temp, pos._node);
         pos._node->_left = temp->_left;
         pos._node->_left->_parent = pos._node;
      }
      if(_Dm->_left != _Dm->_right) {
         _root = _Dm->_left;
         _Dm->_right =_Dm->_left;
      }
      --_size;
      delete temp;
      return true;
   }
   bool erase(const T& x) {
      if (empty()) return false;
      BSTreeNode<T>* temp = _root;
      while (temp->_data != x) {
         if (x < temp->_data) {
            temp = temp->_left;
         }
         else if (x > temp->_data) {
            temp = temp->_right;
         }
         if (temp == 0) return false;
      }
      erase(iterator(temp));
      return true;
   }

   void clear() {
      for (iterator it = begin(); it != end(); ) {
         erase(it++);
      }
   }

private:
   size_t _size;
   BSTreeNode<T>* _root, * _Dm;

   void Transplant(BSTreeNode<T>* z, BSTreeNode<T>* y) {
      y->_parent = z->_parent;
      if (z == z->_parent->_left) 
         z->_parent->_left = y;
      else
         z->_parent->_right = y;
   }
};

#endif // BST_H
   
