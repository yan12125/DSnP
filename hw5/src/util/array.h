/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   Array() : _data(0), _size(0), _capacity(0) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const 
      {
         return *node;
      }
      T& operator * () { return (*_node); }
      iterator& operator ++ () // prefix
      {
         this->node++;
         return *this;
      }
      iterator operator ++ (int) // suffix
      {
         iterator temp = *this;
         this->node++;
         return temp;
      }
      iterator& operator -- ()
      {
         this->node--;
         return (*this);
      }
      iterator operator -- (int)
      {
         iterator temp = *temp;
         this->node--;
         return temp;
      }

      iterator operator + (int i) const 
      {
         iterator temp = *this;
         this->node += i;
         return temp; 
      }
      iterator& operator += (int i) 
      {
         this->node += i;
         return (*this);
      }

      iterator& operator = (const iterator& i) 
      {
         this->node = i.node;
         return (*this);
      }

      bool operator != (const iterator& i) const 
      {
         return (this->node != i.node);
      }
      bool operator == (const iterator& i) const 
      {
         return (this->node == i.node); 
      }

   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const 
   {
      return iterator(data);
   }
   iterator end() const 
   {
      return iterator(data + _size); 
   }
   bool empty() const 
   {
      return (_size == 0);
   }
   size_t size() const
   {
      return _size;
   }

   T& operator [] (size_t i) 
   {
      return _data[i];
   }
   const T& operator [] (size_t i) const 
   {
      return _data[i];
   }

   void pop_front()
   {
      if(_size != 0)
      {
         for(size_t i = 0 ; i<_size-1 ; i++)
         {
            _data[i] = _data[i+1];
         }
         _size--;--
      }
   }
   void pop_back() { }

   bool erase(iterator pos) { return false; }
   bool erase(const T&x) { return false; }
   bool insert(const T& x) { return false; }

   void clear() { }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   T*           _data;
   size_t       _size;       // number of valid elements
   size_t       _capacity;   // max number of elements

   // [OPTIONAL TODO] Helper functions;
};

#endif // ARRAY_H
