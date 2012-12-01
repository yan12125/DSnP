/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-2012 LaDs(III), GIEE, NTU, Taiwan ]
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
      const T& operator * () const
      {
         return _node->data;
      }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () // prefix
      {
         this->_node = this->_node->_next;
         return *(this);
      }
      iterator operator ++ (int)
      {
         iterator temp = *this;
         this->_node = this->_node->_next;
         return temp;
      }
      iterator& operator -- ()
      {
         this->_node = this->_node->_prev;
         return *(this);
      }
      iterator operator -- (int) 
      {
         iterator temp = *this;
         this->_node = this->_node->_prev;
         return temp;
      }

      iterator& operator = (const iterator& i)
      {
         this->_node = i._node;
         return *(this);
      }

      bool operator != (const iterator& i) const
      {
         return this->_node != i._node;
      }
      bool operator == (const iterator& i) const
      {
         return this->_node == i._node;
      }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const
   {
      return iterator(_head);
   }
   iterator end() const
   {
      return iterator(_head->_prev); // dummy node
   }
   bool empty() const
   {
      return _head->_next == _head;
   }
   size_t size() const
   {
      int count = 0;
      DListNode<T>* cur = _head;
      while((cur = cur->_next) != _head)
      {
         count++;
      }
      return count;
   }

   void pop_front()
   {
      if(_head->_next != _head) // not empty
      {
         DListNode<T> *dummy = _head->_prev, *oldHead = _head;
         _head = _head->_next;
         _head->_prev = dummy;
         dummy->_next = _head;
         delete oldHead;
      }
   }
   void pop_back()
   {
      if(_head->_next != _head) // not empty
      {
         DListNode<T>* dummy = _head->_prev;
         DListNode<T>* oldEnd = dummy->_prev;
         oldEnd->_prev->_next = dummy;
         dummy->_prev = oldEnd->_prev;
         delete oldEnd;
      }
   }

   // return false if nothing to erase
   bool erase(iterator pos)
   {
      if(_head->_next == _head) // empty
      {
         return false;
      }
      else
      {
         eraseInternal(pos._node);
         return true;
      }
   }
   bool erase(const T& x)
   {
      DListNode<T>* cur = _head;
      while((cur = cur->_next) != _head) // not dummy
      {
         if(cur->_data == x)
         {
            DListNode<T>* tempCur = cur->_prev;
            eraseInternal(cur);
            cur = tempCur;
         }
      }
      return false;
   }
   // return false if insertion fails
   bool insert(const T& x) { return false; }

   void clear() { } // delete all nodes except for the dummy node

private:
   DListNode<T>*  _head;  // = dummy node if list is empty

   // [OPTIONAL TODO] helper functions; 
   void eraseInternal(DListNode<T>* ptr)
   {
      ptr->_prev->_next = ptr->_next;
      ptr->_next->_prev = ptr->_prev;
      delete ptr;
      return;
   }
};

#endif // DLIST_H
